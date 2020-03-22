// UDiskAutoCopy.cpp : �������̨Ӧ�ó������ڵ㡣

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
#define DISK_NAME_LEN		4						//"D:\\"��ת���һ��\��
#define BACK_DIR				"D:\\U"
#define DIR_MAX_LEN			260		

static bool GlobalFlag = false;
//#define DRIVE_REMOVABLE	2
//#define DRIVE_FIXED			3
//#define DRIVE_REMOTE		4
//#define DRIVE_CDROM			5

//�ݹ�˼·��ɨ���Ŀ¼�������ļ���
//������ļ�������CopyFile()��������ļ��У������ݹ飬��Ŀ���ļ������½��ļ���(folder)�����������ļ����С�
int CopyDir(char szSrcPath[], char szDstPath[]) {	
	WIN32_FIND_DATA stWinFileDate;//�洢���ļ��������ļ��У���Ϣ
	HANDLE hd;//���

	char szFindDir[DIR_MAX_LEN] = { 0 };//Ҫ���ҵ�Ŀ¼
	char szNextSrc[DIR_MAX_LEN] = { 0 };//��Ŀ¼
	char szNextDst[DIR_MAX_LEN] = { 0 };//��Ŀ¼Ҫ������Ŀ���ļ���

	//*��ʾ��Ŀ¼�������ļ�
	_snprintf_c(szFindDir, DIR_MAX_LEN, "%s\\*", szSrcPath);
	//���ļ���һ��Ŀ¼dir�������ļ�������Ϣ
	//dir = D:\* ������Ŀ¼�������ļ�
	hd = FindFirstFile(szFindDir, &stWinFileDate);//��һ���ļ���Ϣ����AllData�����ص�hdΪ�������������һ���ļ�
	if (hd == INVALID_HANDLE_VALUE){//hd����ʧ��
		return ERR;
	}
	//��ʼ����
	//filename����.����ʾ��ǰĿ¼����..����ʾ�ϼ�Ŀ¼
	//D:\A\..\aa �൱��D:\aa
	while (1) {
		if (strcmp(stWinFileDate.cFileName, ".") != 0 && strcmp(stWinFileDate.cFileName, "..") != 0) {
			printf("%s\n", stWinFileDate.cFileName);
			if (stWinFileDate.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {//�ж��Ƿ�Ϊ�ļ���FILE_ATTRIBUTE_DIRECTORY
				//���ļ���
				memset(szNextDst, 0, DIR_MAX_LEN);//Ŀ¼��0
				_snprintf_c(szNextDst, DIR_MAX_LEN, "%s\\%s", szDstPath, stWinFileDate.cFileName);//��������
				
				mkdir(szNextDst);//������һ���ļ�

				memset(szNextSrc, 0, DIR_MAX_LEN);
				_snprintf_c(szNextSrc, DIR_MAX_LEN, "%s\\%s", szSrcPath, stWinFileDate.cFileName);
				//�Ը��ļ���Ϊ������ʼ�ݹ�
				CopyDir(szNextSrc, szNextDst);
			}
			else {//���ļ�
				memset(szNextDst, 0, DIR_MAX_LEN);
				_snprintf_c(szNextDst, DIR_MAX_LEN, "%s\\%s", szDstPath, stWinFileDate.cFileName);
				
				memset(szNextSrc, 0, DIR_MAX_LEN);
				_snprintf_c(szNextSrc, DIR_MAX_LEN, "%s\\%s", szSrcPath, stWinFileDate.cFileName);
				//�������ļ�
				//printf("copyFile:[%s]->[%s]\n", szNextSrc, szNextDst);
				CopyFile(szNextSrc, szNextDst, false);
			}
		}
		if (!FindNextFile(hd, &stWinFileDate))//ͨ���������һ���ļ�������bool��true��ʾ�ҵ���
		{
			break;
		}
	}
	FindClose(hd);//�رվ��
	return OK;
}

int getDisk() {
	DWORD drv = GetLogicalDrives();//��ȡ�����е������̷���drv��32��bitλ��26����ĸռ26λ�����ڼ�1
	char szDisk[DISK_NAME_LEN] = { 0 };//Դ�ļ�
	int iDrvType;//�ļ�����
	char szDstPath[DIR_MAX_LEN] = { 0 };//Ŀ�ĵ�ַ

	if (drv == 0) {//�������0����ʾ���������ﲻ����û�̷�
		return ERR;
	}
	cout << "�����̷� = " << hex << drv << endl;//Ĭ�Ϸ���ʮ���ơ�hex��ʾת��Ϊʮ������
	for (int i = 0; i < DISK_MAX; i++) {
		//�ж�drv�ĵ�i������λ�Ƿ�Ϊ0
		if (drv&(1 << i)) {//��Ϊ0
			//�ҵ��̷�����A��+i
			_snprintf_c(szDisk, DISK_NAME_LEN, "%c:\\", 'A' + i);//����ǣ��ʹ�ӡ��Ӧ�̷���ĸ������szDisk
			iDrvType = GetDriveType(szDisk);//�õ��̷����ͣ��ж��Ƿ�ΪU��   *�ַ�����Ϊ���ֽ��ַ�
			printf("%c��Type = %d\n", 'A' + i, iDrvType);

			if (iDrvType == DRIVE_REMOVABLE) {//�ж���U�����ͣ�2��
				//��U�̡���ȡU��״̬���Ƿ���Դ��ˣ���
				if (GetVolumeInformation(szDisk, 0, 0, 0, 0, 0, 0, 0)) {//GetVolumeInformation()����trueʱ��ʾ׼������
					GlobalFlag = true;
					//û�п���Ŀ���ļ��оʹ���

					cout << "��⵽U�̣�" << szDisk << " ������ʼ������Ŀ���ļ��У�" << BACK_DIR << endl;
					string folderPath = "D:\\U";
					if (GetFileAttributesA(folderPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
						bool flag = CreateDirectory(folderPath.c_str(), NULL);
						if (flag) {
							cout << "D:\\U�����ɹ�." << endl;
						}	
						else {
							cout << "D:\\U�Ѵ���." << endl;
						}
					}

					//�ݹ鿽��
					_snprintf_c(szDstPath, DIR_MAX_LEN, "%s\\%d", BACK_DIR, time(0));
					//��BACK_DIR���½�һ����ʱ�䣨time(0)���������ļ���
					mkdir(szDstPath);

					//ִ������Ŀ¼������
					CopyDir(szDisk, szDstPath);//U���̷���Ŀ���ļ���
					cout << "������ɣ�" << endl;
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
			cout << "��ѯ�����..." << endl;
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


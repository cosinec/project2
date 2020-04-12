#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <winsock2.h>
#include <direct.h>
#include <io.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_SIZE 30
#define ONE_PAGE 4096
struct FileHead
{
	char str[260];
	int size;
};

SOCKET m_Client;

void RecvFile();



int main(int argc, char* argv[])
{
	//��ʼ��WSA  
	cout << "server��" << endl;
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//�����׽���  
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	//��IP�Ͷ˿�  
	sockaddr_in sin;
	sin.sin_family = AF_INET;


	sin.sin_port = htons(8888);//��Ҫ�����Ķ˿�
	//sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_addr.S_un.S_addr = inet_addr("192.168.0.104");//��Ҫ�󶨵����ص��ĸ�IP��ַ
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)//���а󶨶���
	{
		printf("bind error !");
	}

	//��ʼ����  
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		return 0;
	}

	//ѭ����������  
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[255];
	while (true)
	{
		printf("�ȴ�����...\n");
		m_Client = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);//������ֱ������tcp�ͻ�������
		if (m_Client == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		printf("���ܵ�һ�����ӣ�%s \r\n", inet_ntoa(remoteAddr.sin_addr));
		printf("׼����������...\n");
		RecvFile();
		closesocket(m_Client);//�ر�socket
	}

	closesocket(slisten);//�رռ���socket
	WSACleanup();//ж��
	system("pause");
	return 0;
}

void RecvFile() {
	int nlen;
	int filesNum;//�ļ�����
	int foldersNum;//�ļ��и���
	FileHead fh;
	//get numbers
	recv(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);
	recv(m_Client, (char*)&filesNum, sizeof(filesNum), 0);

	//�ڿ����Ƿ�ɾȥ
	char str[MAX_SIZE] = { 0 };//'yes'
	cout << "�Ƿ�Ҫ�����ļ�(yes/no):" << endl;
	cin >> str;
	send(m_Client, str, sizeof(str), 0);

	char szPath[MAX_SIZE] = { 0 };
	cout << "������Ҫ�洢��·��" << endl;
	cin >> szPath;
	strcat(szPath, "\\");
	for (int i = 0; i < foldersNum; i++)
	{
		//��������ļ���·��
		char folderPath[MAX_SIZE] = { 0 };
		char folderName[10] = { 0 };
		recv(m_Client, folderName, 10, 0);
		strcpy(folderPath, szPath);
		strcat(folderPath, folderName);

		//�ж��ļ����Ƿ���ڣ��������������ļ���
		if (0 != access(folderPath, 0))
		{
			mkdir(folderPath);
		}
	}
	for (int i = 0; i < filesNum; i++)
	{
		nlen = recv(m_Client, (char*)&fh, sizeof(fh), 0);//����ļ���Ϣ
		char szPathName[MAX_SIZE] = { 0 };
		sprintf(szPathName, "%s%s", szPath, fh.str);//ƴ��·�����ļ���
		cout << szPathName << endl;
		fstream fs;
		fs.open(szPathName, fstream::out | fstream::binary | fstream::trunc);//�Կ��ļ�����ʽ��
		int FileSize = fh.size;
		int len;
		char content[ONE_PAGE] = { 0 };
		while (FileSize)
		{
			if (FileSize < ONE_PAGE)
				len = recv(m_Client, content, FileSize, 0);
			else
				len = recv(m_Client, content, ONE_PAGE, 0);
			if (len < 0) {
				cout << "����ʧ��";
				break;
			}
			fs.write(content, len);
			FileSize -= len;
		}
		fs.close();
	}
	cout << "����ɹ�����ע����գ�\n";
	system("pause");
}
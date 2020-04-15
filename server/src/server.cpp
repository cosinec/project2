#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#ifdef _WIN32
#include <winsock2.h>
#include <direct.h>
#include <io.h>
#elif linux
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <unistd.h> 
#define SOCKET int
#endif

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_PATH 260
#define MAX_SIZE 50
#define ONE_PAGE 262144//256*1024
struct FileHead
{
	char str[MAX_PATH];
	int size;
};

SOCKET m_Client;

void RecvFile();

int main(int argc, char* argv[])
{
	//��ʼ��WSA  
	cout << "server��" << endl;
#ifdef _WIN32
	const char on = 0;
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
		cout << "socket error !" << endl;
		return 0;
	}
#elif linux
	int on = 1;
	int slisten;
	//�����׽���
	slisten = socket(PF_INET, SOCK_STREAM,0);
	if (slisten < 0)
	{
		cout << "Create Socket Failed";
		exit(-1);
	}
#endif
	if (setsockopt(slisten, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		cout << "setsockopt failed " << endl;
		exit(-1);
	}
	//��IP�Ͷ˿�  
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);//��Ҫ�����Ķ˿�
							   //sin.sin_addr.S_un.S_addr = INADDR_ANY;
#ifdef _WIN32
	sin.sin_addr.S_un.S_addr = inet_addr("192.168.1.109");//��Ҫ�󶨵����ص��ĸ�IP��ַ
#elif linux
	sin.sin_addr.s_addr = inet_addr("192.168.1.105");//��Ҫ�󶨵����ص��ĸ�IP��ַ
#endif
	if (bind(slisten, (struct sockaddr *)&sin, sizeof(sin)) == -1)//���а󶨶���
	{
		cout << "bind error !" << endl;
		exit(-1);
	}
	//��ʼ����  
	if (listen(slisten, 5) == -1)
	{
		cout << "listen error !" << endl;
		exit(-1);
	}
	//��������  
	struct sockaddr_in remoteAddr;
#ifdef _WIN32
	int nAddrlen = sizeof(remoteAddr);
#elif linux
	socklen_t nAddrlen = sizeof(remoteAddr);
#endif
	char revData[255];
	printf("Waiting for connection...\n");
	m_Client = accept(slisten, (struct sockaddr *)&remoteAddr, &nAddrlen);//������ֱ������tcp�ͻ�������
	if (m_Client == -1)
	{
		cout << "accept error !" << endl;
		exit(-1);
	}
	cout << "Received a connection��" << inet_ntoa(remoteAddr.sin_addr) << endl;
	cout << "Preparing for receive file...\n";
	RecvFile();
	
#ifdef _WIN32
	closesocket(slisten);//�رռ���socket
	closesocket(m_Client);//�ر�socket
	WSACleanup();//ж��
#elif linux
	close(slisten);
	close(m_Client);
#endif
	return 0;
}

//\��/�Ļ���ת��
void DealSlash(char* path)
{
#ifdef _WIN32
	char *p = strchr(path, '/');
	if (p - path != 0)
	{
		for (int i = 0; i < strlen(path); i++)
			if (path[i] == '/')
				path[i] = '\\';
	}
#elif linux
	char *p = strchr(path, '\\');
	if (p - path != 0)
	{
		for (int i = 0; i < strlen(path); i++)
			if (path[i] == '\\')
				path[i] = '/';
	}
#endif
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
	cout << "whether to receive files(yes/no)?" << endl;
	cin >> str;
	send(m_Client, str, sizeof(str), 0);

	char szPath[MAX_PATH] = { 0 };
	cout << "Please enter the saved path:" << endl;
	cin >> szPath;
#ifdef _WIN32
	strcat(szPath, "\\");
#elif linux
	strcat(szPath, "/");
#endif
	for (int i = 0; i < foldersNum; i++)
	{
		//��������ļ���·��
		char folderPath[MAX_PATH] = { 0 };
		char folderName[MAX_SIZE/2] = { 0 };
		recv(m_Client, folderName, MAX_SIZE/2, 0);
		strcpy(folderPath, szPath);
		DealSlash(folderName);
		strcat(folderPath, folderName);

		//�ж��ļ����Ƿ���ڣ��������������ļ���
		if (0 != access(folderPath, 0))
		{
#ifdef _WIN32
			mkdir(folderPath);
#elif linux
			strcat(folderPath, "/");
			mkdir(folderPath,0755);
#endif
		}
	}
	for (int i = 0; i < filesNum; i++)
	{
		nlen = recv(m_Client, (char*)&fh, sizeof(fh), 0);//����ļ���Ϣ
		char szPathName[MAX_PATH] = { 0 };
		DealSlash(fh.str);
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
				exit(-1);
			}
			fs.write(content, len);
			FileSize -= len;
		}
		fs.close();
	}
	cout << "The transfer was successful! Please check!" << endl;
	system("pause");
}
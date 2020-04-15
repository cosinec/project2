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
	//初始化WSA  
	cout << "server端" << endl;
#ifdef _WIN32
	const char on = 0;
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//创建套接字  
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		cout << "socket error !" << endl;
		return 0;
	}
#elif linux
	int on = 1;
	int slisten;
	//创建套接字
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
	//绑定IP和端口  
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);//需要监听的端口
							   //sin.sin_addr.S_un.S_addr = INADDR_ANY;
#ifdef _WIN32
	sin.sin_addr.S_un.S_addr = inet_addr("192.168.1.109");//需要绑定到本地的哪个IP地址
#elif linux
	sin.sin_addr.s_addr = inet_addr("192.168.1.105");//需要绑定到本地的哪个IP地址
#endif
	if (bind(slisten, (struct sockaddr *)&sin, sizeof(sin)) == -1)//进行绑定动作
	{
		cout << "bind error !" << endl;
		exit(-1);
	}
	//开始监听  
	if (listen(slisten, 5) == -1)
	{
		cout << "listen error !" << endl;
		exit(-1);
	}
	//接收数据  
	struct sockaddr_in remoteAddr;
#ifdef _WIN32
	int nAddrlen = sizeof(remoteAddr);
#elif linux
	socklen_t nAddrlen = sizeof(remoteAddr);
#endif
	char revData[255];
	printf("Waiting for connection...\n");
	m_Client = accept(slisten, (struct sockaddr *)&remoteAddr, &nAddrlen);//阻塞，直到有新tcp客户端连接
	if (m_Client == -1)
	{
		cout << "accept error !" << endl;
		exit(-1);
	}
	cout << "Received a connection：" << inet_ntoa(remoteAddr.sin_addr) << endl;
	cout << "Preparing for receive file...\n";
	RecvFile();
	
#ifdef _WIN32
	closesocket(slisten);//关闭监听socket
	closesocket(m_Client);//关闭socket
	WSACleanup();//卸载
#elif linux
	close(slisten);
	close(m_Client);
#endif
	return 0;
}

//\和/的互相转换
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
	int filesNum;//文件个数
	int foldersNum;//文件夹个数
	FileHead fh;
	//get numbers
	recv(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);
	recv(m_Client, (char*)&filesNum, sizeof(filesNum), 0);

	//在考虑是否删去
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
		//获得完整文件夹路径
		char folderPath[MAX_PATH] = { 0 };
		char folderName[MAX_SIZE/2] = { 0 };
		recv(m_Client, folderName, MAX_SIZE/2, 0);
		strcpy(folderPath, szPath);
		DealSlash(folderName);
		strcat(folderPath, folderName);

		//判断文件夹是否存在，不存在则生成文件夹
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
		nlen = recv(m_Client, (char*)&fh, sizeof(fh), 0);//获得文件信息
		char szPathName[MAX_PATH] = { 0 };
		DealSlash(fh.str);
		sprintf(szPathName, "%s%s", szPath, fh.str);//拼接路径和文件名
		cout << szPathName << endl;
		fstream fs;
		fs.open(szPathName, fstream::out | fstream::binary | fstream::trunc);//以空文件的形式打开
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
				cout << "传输失败";
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
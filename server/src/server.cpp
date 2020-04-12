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
	//初始化WSA  
	cout << "server端" << endl;
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
		printf("socket error !");
		return 0;
	}

	//绑定IP和端口  
	sockaddr_in sin;
	sin.sin_family = AF_INET;


	sin.sin_port = htons(8888);//需要监听的端口
	//sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_addr.S_un.S_addr = inet_addr("192.168.0.104");//需要绑定到本地的哪个IP地址
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)//进行绑定动作
	{
		printf("bind error !");
	}

	//开始监听  
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		return 0;
	}

	//循环接收数据  
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[255];
	while (true)
	{
		printf("等待连接...\n");
		m_Client = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);//阻塞，直到有新tcp客户端连接
		if (m_Client == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		printf("接受到一个连接：%s \r\n", inet_ntoa(remoteAddr.sin_addr));
		printf("准备接收数据...\n");
		RecvFile();
		closesocket(m_Client);//关闭socket
	}

	closesocket(slisten);//关闭监听socket
	WSACleanup();//卸载
	system("pause");
	return 0;
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
	cout << "是否要接受文件(yes/no):" << endl;
	cin >> str;
	send(m_Client, str, sizeof(str), 0);

	char szPath[MAX_SIZE] = { 0 };
	cout << "请输入要存储的路径" << endl;
	cin >> szPath;
	strcat(szPath, "\\");
	for (int i = 0; i < foldersNum; i++)
	{
		//获得完整文件夹路径
		char folderPath[MAX_SIZE] = { 0 };
		char folderName[10] = { 0 };
		recv(m_Client, folderName, 10, 0);
		strcpy(folderPath, szPath);
		strcat(folderPath, folderName);

		//判断文件夹是否存在，不存在则生成文件夹
		if (0 != access(folderPath, 0))
		{
			mkdir(folderPath);
		}
	}
	for (int i = 0; i < filesNum; i++)
	{
		nlen = recv(m_Client, (char*)&fh, sizeof(fh), 0);//获得文件信息
		char szPathName[MAX_SIZE] = { 0 };
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
				break;
			}
			fs.write(content, len);
			FileSize -= len;
		}
		fs.close();
	}
	cout << "传输成功！请注意查收！\n";
	system("pause");
}
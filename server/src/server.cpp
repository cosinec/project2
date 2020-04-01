#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_SIZE 10
#define ONE_PAGE 4096
struct FileHead
{
	char str[260];
	int size;
};

SOCKET m_Client;

void SendFile();

int main(int argc, char* argv[])
{
	//初始化WSA  
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
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
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
		m_Client = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
		if (m_Client == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		printf("接受到一个连接：%s \r\n", inet_ntoa(remoteAddr.sin_addr));
		SendFile();
		closesocket(m_Client);
	}

	closesocket(slisten);
	WSACleanup();
	return 0;
}

void SendFile() {
	//1.发送文件头
	char path[260] = { 0 };
	cout << "请输入文件路径" << endl;
	cin >> path;
	//截取文件名
	char* ptemp = path;
	while (*ptemp++ != '\0');
	while (*(--ptemp) != '\\');
	ptemp++;

	fstream fs;
	fs.open(path, fstream::in | fstream::binary);
	fs.seekg(0, fstream::end);//以最后的位置为基准不偏移
	int nlen = fs.tellg();//取得文件大小
	fs.seekg(0, fstream::beg);

	FileHead fh;
	fh.size = nlen;
	memcpy(fh.str, ptemp, MAX_PATH);
	nlen = send(m_Client, (char*)&fh, sizeof(fh), 0);
	//2,.如果接受到的内容为yes
	char  szResult[MAX_SIZE] = { 0 };
	recv(m_Client, szResult, sizeof(szResult), 0);
	char szBuf[ONE_PAGE] = { 0 };
	if (0 == strcmp(szResult, "yes"))
	{
		//读文件
		while (!fs.eof())
		{
			fs.read(szBuf, ONE_PAGE);
			int len = fs.gcount();
			//if(len == 0 ) break;
			send(m_Client, szBuf, len, 0);
		}
		cout << "成功发送！请接收了再继续";
		system("pause");
	}
	//3.关闭文件流
	fs.close();
}
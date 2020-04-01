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

void RecvFile();

int main()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}
	//while(true){
	m_Client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_Client == INVALID_SOCKET)
	{
		printf("invalid socket!");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = inet_addr("192.168.1.109");
	if (connect(m_Client, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //连接失败 
		printf("connect error !");
		closesocket(m_Client);
		return 0;
	}
	RecvFile();
	closesocket(m_Client);
	//}


	WSACleanup();
	return 0;

}

/*void RecvFile() {
	cout << "start recv!" << endl;
	const int bufferSize = 1024;
	char buffer[bufferSize] = { 0 };
	int readLen = 0;
	string desFileName = "new_in";
	ofstream desFile;
	desFile.open(desFileName.c_str(), ios::binary);
	if (!desFile)
	{
		return;
	}
	do
	{
		readLen = recv(m_Client, buffer, bufferSize, 0);
		if (readLen == 0)
		{
			break;
		}
		else
		{
			desFile.write(buffer, readLen);
		}
	} while (true);
	desFile.close();
}*/
void RecvFile() {
	char str[1024] = { 0 };
	int nlen;
	FileHead fh;
	nlen = recv(m_Client, (char*)&fh, sizeof(fh), 0);
	cout << "是否要接受文件" << endl;
	cin >> str;
	send(m_Client, str, sizeof(str), 0);
	char szPath[MAX_SIZE] = { 0 };
	cout << "请输入要存储的路径" << endl;
	cin >> szPath;
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
		len = recv(m_Client, content, ONE_PAGE, 0);
		if (len > 0)
		{
			fs.write(content, len);
			FileSize -= len;
		}
	}
	fs.close();
}
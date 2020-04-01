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
	//��ʼ��WSA  
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
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
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
		m_Client = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
		if (m_Client == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		printf("���ܵ�һ�����ӣ�%s \r\n", inet_ntoa(remoteAddr.sin_addr));
		SendFile();
		closesocket(m_Client);
	}

	closesocket(slisten);
	WSACleanup();
	return 0;
}

void SendFile() {
	//1.�����ļ�ͷ
	char path[260] = { 0 };
	cout << "�������ļ�·��" << endl;
	cin >> path;
	//��ȡ�ļ���
	char* ptemp = path;
	while (*ptemp++ != '\0');
	while (*(--ptemp) != '\\');
	ptemp++;

	fstream fs;
	fs.open(path, fstream::in | fstream::binary);
	fs.seekg(0, fstream::end);//������λ��Ϊ��׼��ƫ��
	int nlen = fs.tellg();//ȡ���ļ���С
	fs.seekg(0, fstream::beg);

	FileHead fh;
	fh.size = nlen;
	memcpy(fh.str, ptemp, MAX_PATH);
	nlen = send(m_Client, (char*)&fh, sizeof(fh), 0);
	//2,.������ܵ�������Ϊyes
	char  szResult[MAX_SIZE] = { 0 };
	recv(m_Client, szResult, sizeof(szResult), 0);
	char szBuf[ONE_PAGE] = { 0 };
	if (0 == strcmp(szResult, "yes"))
	{
		//���ļ�
		while (!fs.eof())
		{
			fs.read(szBuf, ONE_PAGE);
			int len = fs.gcount();
			//if(len == 0 ) break;
			send(m_Client, szBuf, len, 0);
		}
		cout << "�ɹ����ͣ���������ټ���";
		system("pause");
	}
	//3.�ر��ļ���
	fs.close();
}
#include <stdio.h>

#include <stdlib.h>

#include <iostream>

#include <string.h>

#include <fstream>

#include <vector>

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
#include "md5.h"



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



//传输文件状态

enum status { Success, Interrupt }s;



SOCKET m_Client;

int flag = 0;

vector<string> files;//存放文件路径

vector<string> folders;//存放文件夹路径

string fileMD5[MAX_SIZE];//存放文件的MD5码

bool RecvFile();



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

	slisten = socket(PF_INET, SOCK_STREAM, 0);

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

	sin.sin_addr.S_un.S_addr = inet_addr("192.168.0.101");//需要绑定到本地的哪个IP地址

#elif linux

	sin.sin_addr.s_addr = inet_addr("192.168.1.105");//需要绑定到本地的哪个IP地址

#endif

	if (bind(slisten, (struct sockaddr*) & sin, sizeof(sin)) == -1)//进行绑定动作

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

	printf("等待连接...\n");

	while (1)

	{

		m_Client = accept(slisten, (struct sockaddr*) & remoteAddr, &nAddrlen);//阻塞，直到有新tcp客户端连接

		if (m_Client == -1)

		{

			cout << "accept error !" << endl;

			exit(-1);

		}

		cout << "接收到一个连接：" << inet_ntoa(remoteAddr.sin_addr) << endl;

		cout << "准备传输文件...\n";

		flag = RecvFile();

		if (flag)

			break;

		else

			cout << "客户端断开连接，请求重新连接";

	}



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

	char* p = strchr(path, '/');

	if (p - path != 0)

	{

		for (int i = 0; i < strlen(path); i++)

			if (path[i] == '/')

				path[i] = '\\';

	}

#elif linux

	char* p = strchr(path, '\\');

	if (p - path != 0)

	{

		for (int i = 0; i < strlen(path); i++)

			if (path[i] == '\\')

				path[i] = '/';

	}

#endif

}

//获取文件的MD5码
string FileDigest(const string& file)

{

	ifstream in(file.c_str());

	if (!in)

		return "";



	MD5 md5;

	std::streamsize length;

	char buffer[1024];

	while (!in.eof()) {

		in.read(buffer, 1024);

		length = in.gcount();

		if (length > 0)

			md5.update(buffer, length);

	}

	in.close();

	return md5.toString();

}

/*获取文件内的所有文件路径（包括子文件夹）*/

bool getFiles(char* path)

{

	char subFolderPath[MAX_PATH];

	char filePath[MAX_PATH];



#ifdef _WIN32

	char folderPath[MAX_SIZE];

	_finddata_t file;

	intptr_t lf;

	sprintf_s(folderPath, "%s*", path);

	//输入文件夹路径

	if ((lf = _findfirst(folderPath, &file)) == -1) {

		cout << folderPath << " not found!!!" << endl;

		return false;

	}

	else {

		do {

			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)

				continue;



			//是文件夹就递归调用函数

			if (file.attrib & _A_SUBDIR)

			{

				//cout << file.name << endl;

				strcpy(subFolderPath, path);

				strcat(subFolderPath, file.name);

				folders.push_back(subFolderPath);

				strcat(subFolderPath, "\\");

				getFiles(subFolderPath);

			}

			//保存完整路径

			else

			{

				sprintf_s(filePath, "%s%s", path, file.name);

				//cout << filePath << endl;

				files.push_back(filePath);

			}

		} while (_findnext(lf, &file) == 0);

	}

	_findclose(lf);

#endif



#ifdef linux

	DIR* dir;

	struct dirent* ptr;

	if ((dir = opendir(path)) == NULL)

	{

		perror("Open dir error...");

		return false;

	}



	while ((ptr = readdir(dir)) != NULL)

	{

		memset(filePath, '\0', sizeof(filePath));

		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)

			continue;

		else if (ptr->d_type == 8) //file

		{

			//cout << "filepath:" << path << ptr->d_name << endl;

			strcpy(filePath, path);

			strcat(filePath, ptr->d_name);

			files.push_back(filePath);

		}

		else if (ptr->d_type == 10)    //link file

		{

			//cout << "filepath:" << path << ptr->d_name << endl;

			continue;

		}

		else if (ptr->d_type == 4)    //文件夹

		{

			memset(subFolderPath, '\0', sizeof(subFolderPath));

			strcpy(subFolderPath, path);

			strcat(subFolderPath, ptr->d_name);

			folders.push_back(subFolderPath);

			strcat(subFolderPath, "/");

			getFiles(subFolderPath);

		}

	}

	closedir(dir);

#endif

	return true;

}

bool RecvFile() {

	int nlen;

	int filesNum;//文件个数

	int foldersNum;//文件夹个数

	FileHead fh;

	//get numbers

	recv(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);

	recv(m_Client, (char*)&filesNum, sizeof(filesNum), 0);



	//在考虑是否删去

	char str[MAX_SIZE] = { 0 };//'yes'

	cout << "是否接收文件(yes/no)?" << endl;

	cin >> str;

	send(m_Client, str, sizeof(str), 0);

	char szPath[MAX_PATH] = { 0 };

	cout << "请输入存储路径:" << endl;

	cin >> szPath;

#ifdef _WIN32

	strcat(szPath, "\\");

#elif linux

	strcat(szPath, "/");

#endif

	getFiles(szPath);

	char severNum[MAX_SIZE];

	sprintf(severNum, "%d.bmp", files.size());

	send(m_Client, severNum, sizeof(severNum), 0);

	cout << "文件夹下已有文件：\n";

	for (int i = 0; i < files.size(); i++)
	{
		cout << files[i].c_str() << "\n";

		fileMD5[i] = FileDigest(files[i]);

		char* temp = const_cast<char*>(fileMD5[i].c_str());

		cout << "MD5码：" << temp << "\n";

		send(m_Client, temp, 32, 0);
	}

	cout << "MD5码发送完毕\n";

	for (int i = 0; i < foldersNum; i++)

	{

		//获得完整文件夹路径

		char folderPath[MAX_PATH] = { 0 };

		char folderName[MAX_SIZE / 2] = { 0 };

		recv(m_Client, folderName, MAX_SIZE / 2, 0);

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

			mkdir(folderPath, 0755);

#endif

		}

	}

	for (int i = 0; i < filesNum; i++)

	{

		nlen = recv(m_Client, (char*)&fh, sizeof(fh), 0);//获得文件信息



		//80%是因为断开连接

		if (nlen == 0)

			return false;

		char szPathName[MAX_PATH] = { 0 };

		DealSlash(fh.str);

		sprintf(szPathName, "%s%s", szPath, fh.str);//拼接路径和文件名



		//输出正在接受的文件路径

		cout << szPathName;

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



			//80%是因为断开连接

			if (len == 0) {

				cout << "传输失败" << endl;

				s = Interrupt;

				send(m_Client, (char*)&s, sizeof(s), 0);

				return false;

			}

			s = Success;

			send(m_Client, (char*)&s, sizeof(s), 0);

			fs.write(content, len);

			FileSize -= len;

		}

		cout << "\t接收完成!" << endl;

		fs.close();

	}

	cout << "传输成功!请注意查收" << endl;

	system("pause");

	return true;

}
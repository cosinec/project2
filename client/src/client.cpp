#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#ifdef linux
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#define SOCKET int
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <direct.h>
#include <io.h>
#pragma comment(lib, "ws2_32.lib")//socket库文件
#endif

using namespace std;

#define MAX_PATH 260
#define MAX_SIZE 50
#define ONE_PAGE 262144//256*1024

struct FileHead
{
	char str[MAX_PATH];
	int size;
};

enum status { Success, Interrupt }s;

SOCKET m_Client;
int host;
char *addr;
bool SendFile();//发送文件夹内所有文件
bool getFiles(char*);//获得文件信息
void reconnect();//重新连接服务器
void closeSocket();//关闭m_Client
vector<string> files;//存放文件路径
vector<string> folders;//存放文件夹路径

int main()
{
	cout << "client端" << endl;
#ifdef _WIN32
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}
#endif

	m_Client = socket(AF_INET, SOCK_STREAM, 0);
	if (m_Client == 0)
	{
		printf("invalid socket!");
		exit(-1);
	}

	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);//需要监听的端口
#ifdef _WIN32
	serAddr.sin_addr.S_un.S_addr = inet_addr("192.168.101.7");
#elif linux
	serAddr.sin_addr.s_addr = inet_addr("192.168.1.105");//需要绑定到本地的哪个IP地址
#endif
	host = 8888;
	addr = "192.168.101.7";
	if (connect(m_Client, (struct sockaddr *)&serAddr, sizeof(serAddr)) == -1)
	{  //连接失败 
		cout << "连接失败!" << endl;
		closeSocket();
		return 0;
	}
	else
		cout << "连接成功!准备传输文件!\n";
	SendFile();
	closeSocket();
#ifdef _WIN32
	WSACleanup();//卸载
#endif
	return 0;
}

void closeSocket()
{
#ifdef _WIN32
	closesocket(m_Client);//关闭监听socket
#elif linux
	close(m_Client);
#endif
}

//重新连接服务器
void reconnect()
{
	cout << "服务器连接中断!重新连接!" << endl;
	closeSocket();
	m_Client = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(host);//需要监听的端口
#ifdef _WIN32
	serAddr.sin_addr.S_un.S_addr = inet_addr(addr);
#elif linux
	serAddr.sin_addr.s_addr = inet_addr("192.168.1.105");//需要绑定到本地的哪个IP地址
#endif
	Sleep(1000);
	if (connect(m_Client, (struct sockaddr *)&serAddr, sizeof(serAddr)) == -1)
	{  //连接失败 
		cout << "connect error !" << endl;
		closeSocket();
		return;
	}
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
			if (file.attrib&_A_SUBDIR)
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
	DIR *dir;
	struct dirent *ptr;
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

bool SendFile() {
	int rec;
	char path[MAX_PATH] = { 0 };
	cout << "请输入文件路径:" << endl;
	cin >> path;
#ifdef _WIN32
	strcat(path, "\\");
#elif linux
	strcat(path, "/");
#endif
	getFiles(path);
	int pathLen = strlen(path);//文件夹路径长度 格式d:\\xx\\xx\\
							   							   //发送文件夹个数和文件个数
	int foldersNum = folders.size();
	send(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);
	int filesNum = files.size();
	send(m_Client, (char*)&filesNum, sizeof(filesNum), 0);
	//如果收到的为yes
	char  szResult[MAX_SIZE] = { 0 };
	recv(m_Client, szResult, sizeof(szResult), 0);

	if (0 == strcmp(szResult, "yes"))
	{
		//传输文件夹名称
		//文件夹名/文件夹名
		for (int i = 0; i < foldersNum; i++)
		{
			char* ptemp = const_cast<char*>(folders[i].c_str());//把string转换成char*
			for (int i = 0; i < pathLen; i++)
				*ptemp++;

			send(m_Client, ptemp, MAX_SIZE / 2, 0);//不想浪费空间
		}

		for (int i = 0; i < filesNum; i++)
		{
			/*截取需要的文件路径部分
			文件夹名/文件.txt*/
			char* ptemp = const_cast<char*>(files[i].c_str());//把string转换成char*
			for (int i = 0; i < pathLen; i++)
				*ptemp++;

			fstream fs;
			fs.open(files[i], fstream::in | fstream::binary);
			fs.seekg(0, fstream::end);//以最后的位置为基准不偏移
			int nlen = fs.tellg();//取得文件大小
			fs.seekg(0, fstream::beg);

			//发送文件信息
			FileHead fh;
			fh.size = nlen;
			memcpy(fh.str, ptemp, MAX_PATH);
			nlen = send(m_Client, (char*)&fh, sizeof(fh), 0);
			char szBuf[ONE_PAGE] = { 0 };
			cout << files[i].c_str();
			//读文件
			while (!fs.eof())
			{
				fs.read(szBuf, ONE_PAGE);
				int len = fs.gcount();

				if (len == 0) 
					return false;
				send(m_Client, szBuf, len, 0);
				rec = recv(m_Client, (char*)s, sizeof(s), 0);
				if (rec <= 0)
				{
					reconnect();
				}
			}
			cout << "\t传输完成!" << endl;
			//关闭文件流
			fs.close();
		}
		cout << "所有文件传输成功，请注意查收" << endl;
	}
#ifdef _WIN32
	system("pause");
#endif
}
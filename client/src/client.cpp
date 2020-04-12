#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <winsock2.h>
#include <vector>
#ifdef linux
#include <unistd.h>
#include <dirent.h>
#endif
#ifdef WIN32
#include <direct.h>
#include <io.h>
#endif
#pragma comment(lib, "ws2_32.lib")//socket库文件

using namespace std;


#define MAX_SIZE 30
#define ONE_PAGE 1024

struct FileHead
{
	char str[260];
	int size;
};

SOCKET m_Client;
void SendFile();
bool getFiles(char*);
vector<string> files;//存放文件路径
vector<string> folders;//存放文件夹路径

int main()
{
	cout << "client端" << endl;
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
	serAddr.sin_port = htons(8888);//需要监听的端口

	serAddr.sin_addr.S_un.S_addr = inet_addr("192.168.0.104");//需要绑定到本地的哪个IP地址
	if (connect(m_Client, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //连接失败 
		printf("connect error !");
		closesocket(m_Client);
		return 0;
	}
	else printf("connect successfully!Preparing for sending file...\n");
	SendFile();
	closesocket(m_Client);//关闭监听socket

	WSACleanup();//卸载
	system("pause");
	return 0;

}

/*获取文件内的所有文件路径（包括子文件夹）*/
bool getFiles(char* path)
{
	char folderPath[MAX_SIZE];
	char subFolderPath[MAX_SIZE];
	char filePath[MAX_SIZE];

#ifdef WIN32
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
				folders.push_back(file.name);
				strcpy(subFolderPath, path);
				strcat(subFolderPath, file.name);
				strcat(subFolderPath, "\\");
				getFiles(subFolderPath);
			}
			//保存完整路径
			else
			{
				sprintf_s(filePath, "%s%s", path, file.name);
				cout << filePath << endl;
				files.push_back(filePath);
			}
		} while (_findnext(lf, &file) == 0);
	}
	_findclose(lf);
#endif

	//linux还没写好
#ifdef linux
	DIR *dir;
	struct dirent *ptr;
	sprintf_s(folderPath, "%s*", path);
	char base[1000];

	if ((dir = opendir(folderPath)) == NULL)
	{
		perror("Open dir error...");
		return false;
	}

	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
			continue;
		else if (ptr->d_type == 8)    ///file
			printf("d_name:%s/%s\n", basePath, ptr->d_name);
		files.push_back(ptr->d_name);
		else if (ptr->d_type == 10)    ///link file
			printf("d_name:%s/%s\n", basePath, ptr->d_name);
		continue;
		else if (ptr->d_type == 4)    ///dir
		{
			memset(base, '\0', sizeof(base));
			strcpy(base, basePath);
			strcat(base, "/");
			strcat(base, ptr->d_nSame);
			readFileList(base);

		}
	}
	closedir(dir);
#endif
	return true;
}

void SendFile() {
	char path[260] = { 0 };
	cout << "请输入文件路径" << endl;
	cin >> path;
	strcat(path, "\\");
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
		for (int i = 0; i < foldersNum; i++)
		{
			send(m_Client, folders[i].c_str(), 10, 0);
		}

		for (int i = 0; i < filesNum; i++)
		{
			//截取需要的文件路径部分
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

			//读文件
			while (!fs.eof())
			{
				fs.read(szBuf, ONE_PAGE);
				int len = fs.gcount();
				if (len == 0) break;
				send(m_Client, szBuf, len, 0);
			}

			//关闭文件流
			fs.close();
		}
	}
	cout << "发送完成!请注意查收!" << endl;
	system("pause");
}
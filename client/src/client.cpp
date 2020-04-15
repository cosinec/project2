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
#pragma comment(lib, "ws2_32.lib")//socket���ļ�
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

SOCKET m_Client;
void SendFile();//�����ļ����������ļ�
bool getFiles(char*);//����ļ���Ϣ
void closeSocket();//�ر�m_Client
vector<string> files;//����ļ�·��
vector<string> folders;//����ļ���·��

int main()
{
	cout << "client��" << endl;
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
	serAddr.sin_port = htons(8888);//��Ҫ�����Ķ˿�
#ifdef _WIN32
	serAddr.sin_addr.S_un.S_addr = inet_addr("192.168.1.109");
#elif linux
	serAddr.sin_addr.s_addr = inet_addr("192.168.1.105");//��Ҫ�󶨵����ص��ĸ�IP��ַ
#endif
	if (connect(m_Client, (struct sockaddr *)&serAddr, sizeof(serAddr)) == -1)
	{  //����ʧ�� 
		cout << "connect error !" << endl;
		closeSocket();
		return 0;
	}
	else
		printf("connect successfully!Preparing for sending file...\n");
	SendFile();
	closeSocket();
#ifdef _WIN32
	WSACleanup();//ж��
#endif
	return 0;
}

void closeSocket()
{
#ifdef _WIN32
	closesocket(m_Client);//�رռ���socket
#elif linux
	close(m_Client);
#endif
}

/*��ȡ�ļ��ڵ������ļ�·�����������ļ��У�*/
bool getFiles(char* path)
{
	char subFolderPath[MAX_PATH];
	char filePath[MAX_PATH];

#ifdef _WIN32
	char folderPath[MAX_SIZE];
	_finddata_t file;
	intptr_t lf;
	sprintf_s(folderPath, "%s*", path);
	//�����ļ���·��
	if ((lf = _findfirst(folderPath, &file)) == -1) {
		cout << folderPath << " not found!!!" << endl;
		return false;
	}
	else {
		do {
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
				continue;

			//���ļ��о͵ݹ���ú���
			if (file.attrib&_A_SUBDIR)
			{
				//cout << file.name << endl;
				strcpy(subFolderPath, path);
				strcat(subFolderPath, file.name);
				folders.push_back(subFolderPath);
				strcat(subFolderPath, "\\");
				getFiles(subFolderPath);
			}
			//��������·��
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
			cout << "filepath:" << path << ptr->d_name << endl;
			strcpy(filePath, path);
			strcat(filePath, ptr->d_name);
			files.push_back(filePath);
		}
		else if (ptr->d_type == 10)    //link file
		{
			cout << "filepath:" << path << ptr->d_name << endl;
			continue;
		}
		else if (ptr->d_type == 4)    //�ļ���
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

void SendFile() {
	char path[MAX_PATH] = { 0 };
	cout << "Please enter the folder path:" << endl;
	cin >> path;
#ifdef _WIN32
	strcat(path, "\\");
#elif linux
	strcat(path, "/");
#endif
	getFiles(path);
	int pathLen = strlen(path);//�ļ���·������ ��ʽd:\\xx\\xx\\
							   							   //�����ļ��и������ļ�����
	int foldersNum = folders.size();
	send(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);
	int filesNum = files.size();
	send(m_Client, (char*)&filesNum, sizeof(filesNum), 0);
	//����յ���Ϊyes
	char  szResult[MAX_SIZE] = { 0 };
	recv(m_Client, szResult, sizeof(szResult), 0);

	if (0 == strcmp(szResult, "yes"))
	{
		//�����ļ�������
		//�ļ�����/�ļ�����
		for (int i = 0; i < foldersNum; i++)
		{
			char* ptemp = const_cast<char*>(folders[i].c_str());//��stringת����char*
			for (int i = 0; i < pathLen; i++)
				*ptemp++;

			send(m_Client, ptemp, MAX_SIZE / 2, 0);//�����˷ѿռ�
		}

		for (int i = 0; i < filesNum; i++)
		{
			/*��ȡ��Ҫ���ļ�·������
			�ļ�����/�ļ�.txt*/
			char* ptemp = const_cast<char*>(files[i].c_str());//��stringת����char*
			for (int i = 0; i < pathLen; i++)
				*ptemp++;

			fstream fs;
			fs.open(files[i], fstream::in | fstream::binary);
			fs.seekg(0, fstream::end);//������λ��Ϊ��׼��ƫ��
			int nlen = fs.tellg();//ȡ���ļ���С
			fs.seekg(0, fstream::beg);

			//�����ļ���Ϣ
			FileHead fh;
			fh.size = nlen;
			memcpy(fh.str, ptemp, MAX_PATH);
			nlen = send(m_Client, (char*)&fh, sizeof(fh), 0);
			char szBuf[ONE_PAGE] = { 0 };

			//���ļ�
			while (!fs.eof())
			{
				fs.read(szBuf, ONE_PAGE);
				int len = fs.gcount();
				if (len == 0) break;
				send(m_Client, szBuf, len, 0);
			}
			//�ر��ļ���
			fs.close();
		}
	}
	cout << "The transfer was successful! Please check" << endl;
#ifdef _WIN32
	system("pause");
#endif
}
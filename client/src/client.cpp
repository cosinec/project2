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
#pragma comment(lib, "ws2_32.lib")//socket���ļ�

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
vector<string> files;//����ļ�·��
vector<string> folders;//����ļ���·��

int main()
{
	cout << "client��" << endl;
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
	serAddr.sin_port = htons(8888);//��Ҫ�����Ķ˿�

	serAddr.sin_addr.S_un.S_addr = inet_addr("192.168.0.104");//��Ҫ�󶨵����ص��ĸ�IP��ַ
	if (connect(m_Client, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //����ʧ�� 
		printf("connect error !");
		closesocket(m_Client);
		return 0;
	}
	else printf("connect successfully!Preparing for sending file...\n");
	SendFile();
	closesocket(m_Client);//�رռ���socket

	WSACleanup();//ж��
	system("pause");
	return 0;

}

/*��ȡ�ļ��ڵ������ļ�·�����������ļ��У�*/
bool getFiles(char* path)
{
	char folderPath[MAX_SIZE];
	char subFolderPath[MAX_SIZE];
	char filePath[MAX_SIZE];

#ifdef WIN32
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
				folders.push_back(file.name);
				strcpy(subFolderPath, path);
				strcat(subFolderPath, file.name);
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

	//linux��ûд��
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
	cout << "�������ļ�·��" << endl;
	cin >> path;
	strcat(path, "\\");
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
		for (int i = 0; i < foldersNum; i++)
		{
			send(m_Client, folders[i].c_str(), 10, 0);
		}

		for (int i = 0; i < filesNum; i++)
		{
			//��ȡ��Ҫ���ļ�·������
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
	cout << "�������!��ע�����!" << endl;
	system("pause");
}
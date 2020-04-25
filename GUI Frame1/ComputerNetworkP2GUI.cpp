#pragma warning(disable:4996)
#include "md5.h"
#include <wx/wxprec.h>    // 预编译头文件
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx\progdlg.h>
#include "wx/notebook.h"
#include <wx/aui/tabmdi.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>

#undef UNICODE
#ifdef _WIN32
#include <winsock2.h>
#include <direct.h>
#include <io.h>
#include <windows.h>
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

//using namespace std;

#define MAX_PATH 260
#define MAX_SIZE 100
#define ONE_PAGE 262144//256*1024
#define MAX_BUFFER 1024
/*===================================先定义的是server服务端口========================================*/
struct FileHead
{
    char str[MAX_PATH] = "";
    int size;
    int num;
    int interpos;
    char md5[33] = "";
};
//传输状态
enum programStatus { Start, Interrupt };
SOCKET m_Client;
int flag = 1;//用于断开连接后的第一次更新状态
int host;//端口号
const char* addr;//地址
SOCKET slisten;

std::vector<string> files;//存放文件路径
std::vector<string> folders;//存放文件夹路径
//string fileMD5[MAX_SIZE];//存放文件的MD5码

/*==================================App开启使用======================================*/
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};
class MyMDIFrame : public wxAuiMDIParentFrame
{
public:
    MyMDIFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& evt);
    void OnAbout(wxCommandEvent& event);



private:
    //void OnHello(wxCommandEvent& event);

    DECLARE_EVENT_TABLE();
};
class ServerFrame :public wxAuiMDIChildFrame
{
public:
    ServerFrame(wxAuiMDIParentFrame* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);


private:
    //void OnHello(wxCommandEvent& event);
    wxStaticText* m_staticText1;
    wxStaticText* writeTxt;
    wxStaticText* portxt;
    wxStaticText* portnumxt;
    wxStaticText* serveripaddress;
    wxListBox* portbox;
    wxTextCtrl* portnumberinput;
    wxTextCtrl* fileinput;
    wxButton* brouseBtn;
    wxButton* generateBtn;
    wxButton* clearBtn;
    wxButton* exitBtn;
    wxTextCtrl* MytestCtrl;
    wxStaticText* statustxt;//状态栏
    wxTextCtrl* StatusCtrl;
    void OnQuitserver(wxCommandEvent& event);
    void OnButtonOK(wxCommandEvent& event);
    void OnGenerate(wxCommandEvent& event);
    void ScoketError(SOCKET slisten);
    void SetSockoptError(SOCKET slisten);
    void Bind(SOCKET slisten, sockaddr_in sin);
    void Listen(SOCKET slisten);
    void m_ClientError();
    void WsaStartup(WORD sockVersion, WSADATA* wsaData);
    void reconnect();
    void DealSlash(char* path);
    void isinterrupt(programStatus& server, programStatus& client);
    bool getFiles(char* path);
    string FileDigest(const string& file);
#ifdef _WIN32
    bool WindowsPathError(intptr_t* lf, char* folderPath, _finddata_t* file);
#elif linux
    void LinuxPathError(char* path, DIR* dir);
#endif
    void OnSetFile(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    DECLARE_EVENT_TABLE();
};
class ClientFrame :public wxAuiMDIChildFrame
{
public:
    ClientFrame(wxAuiMDIParentFrame* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);


private:
    //void OnHello(wxCommandEvent& event);
    wxStaticText* m_staticText1;
    wxStaticText* writeTxt;
    wxStaticText* portxt;
    wxStaticText* portnumxt;
    wxStaticText* clientipaddress;
    wxListBox* portbox;
    wxTextCtrl* portnumberinput;
    wxButton* brouseBtn;
    wxButton* generateBtn;
    wxButton* clearBtn;
    wxButton* exitBtn;
    wxTextCtrl* MytestCtrl;
    wxTextCtrl* fileinput;
    wxStaticText* statustxt;//状态栏
    wxTextCtrl* StatusCtrl;
    void OnQuitclient(wxCommandEvent& event);
    void OnButtonOK(wxCommandEvent& event);
    void OnGenerate(wxCommandEvent& event);
    void closeSocket();
    void connectError(sockaddr_in serAddr);
    void reconnect();
    void isinterrupt(programStatus& server, programStatus& client);
    bool SendFile();
    string FileDigest(const string& file);
    bool getFiles(char* path);
#ifdef _WIN32
    bool WindowsPathError(intptr_t* lf, char* folderPath, _finddata_t* file);
#else linux  
    void LinuxPathError(char* path, DIR* dir);
#endif
    void OnSetFile(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    DECLARE_EVENT_TABLE();
};
// 事件标识符
enum
{
    ID_Clear = 1,
    ID_SetFile = 2,
    ID_Generate = 3,
    ID_MYCHILD1 = 4,
    ID_MYCHILD2 = 5,
    ID_Quitserver = 6,
    ID_Quitclient = 7
};


/*============================事件表定义区域=============================*/

// 定义事件表
wxBEGIN_EVENT_TABLE(MyMDIFrame, wxAuiMDIParentFrame)
//EVT_MENU(ID_Hello, MyMDIFrame::OnHello)
EVT_MENU(wxID_EXIT, MyMDIFrame::OnExit)
EVT_MENU(wxID_ABOUT, MyMDIFrame::OnAbout)
EVT_CLOSE(OnClose) /// 注册窗口关闭消息处理函数


wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ServerFrame, wxAuiMDIChildFrame)
EVT_BUTTON(ID_Clear, ServerFrame::OnClear)
EVT_BUTTON(ID_SetFile, ServerFrame::OnSetFile)
EVT_BUTTON(wxID_OK, ServerFrame::OnButtonOK)
EVT_BUTTON(ID_Generate, ServerFrame::OnGenerate)
EVT_BUTTON(ID_Quitclient, ServerFrame::OnQuitserver)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ClientFrame, wxAuiMDIChildFrame)

wxEND_EVENT_TABLE()


// 应用的入口
wxIMPLEMENT_APP(MyApp);


// 应用的Init事件句柄
bool MyApp::OnInit()
{
    int max = 500;
    MyMDIFrame* frame = new MyMDIFrame("文件接收系统", wxPoint(300, 150), wxSize(530, 350));
    SetTopWindow(frame);
    frame->Show(true);
    ServerFrame* serverframe = new ServerFrame(
        frame, ID_MYCHILD1, wxT("server port"));
    ClientFrame* clientframe = new ClientFrame(
        frame, ID_MYCHILD2, wxT("client port"));
    frame->Show(true);
    serverframe->Show(true);
    clientframe->Show(true);
    return true;

}

/*============================框架设计区域===============================*/


// 主框架的初始化,以及一系列的操作
MyMDIFrame::MyMDIFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxAuiMDIParentFrame(NULL, wxID_ANY, title, pos, size)
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Clear, "&清除代码...\tCtrl-H",
        "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(ID_SetFile, "&浏览传输文件...\tCtrl-R", "show value");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "about...\tF1");
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("欢迎使用程序，如果有疑问，可以按快捷键F1或者点击菜单栏的help进行查找操作。");
    SetBackgroundColour("yellow");
    // 创建图片列表
    // 创建页面

}
/*搭建的server端口的框架*/
/*==========================server端口=============================*/
ServerFrame::ServerFrame(wxAuiMDIParentFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxAuiMDIChildFrame(parent, id, title, pos, size, style) {
    wxFont font(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD, false);
    wxFont tablefont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL, false);
    wxFont tablefont2(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL, false);
    m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("请在下方执行想要的操作:"), wxPoint(120, 5), wxDefaultSize, 0);
    m_staticText1->SetFont(font);
    portxt = new wxStaticText(this, wxID_ANY, wxT(" 当前端口为 :"), wxPoint(10, 35), wxDefaultSize, 0);
    portxt->SetFont(tablefont);
    portbox = new wxListBox(this, wxID_ANY, wxPoint(115, 34), wxSize(90, 25), NULL, 0);
    //portbox->SetExtraStyle(wxLB_SINGLE);
    portbox->SetFont(tablefont2);
    portbox->Append("服务器端");

    portnumxt = new wxStaticText(this, wxID_ANY, wxT("请输入端口号:"), wxPoint(225, 37), wxDefaultSize, 0);
    portnumxt->SetFont(tablefont);
    portnumberinput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(340, 35), wxSize(133, -1), 0);
    writeTxt = new wxStaticText(this, wxID_ANY, wxT("选择文件夹路径:"), wxPoint(10, 72), wxDefaultSize, 0);
    writeTxt->SetFont(tablefont);
    fileinput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(133, 70), wxSize(252, -1), 0);

    brouseBtn = new wxButton(this, wxID_ANY, wxT("浏览"), wxPoint(390, 70), wxSize(85, 25), 0);
    brouseBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnSetFile, this);
    //bSizer4->Add(m_textCtrl8, 0, wxALL, 5);
    
    serveripaddress = new wxStaticText(this, wxID_ANY, wxT("请输入本机IP地址:"), wxPoint(10, 113), wxDefaultSize, 0);
    serveripaddress->SetFont(tablefont);
   
    MytestCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(165, 110), wxSize(220, -1), 0);
    
    statustxt = new wxStaticText(this, wxID_ANY, wxT("当前状态："), wxPoint(10, 150), wxDefaultSize, 0);
    statustxt->SetFont(tablefont);
    StatusCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(135, 147), wxSize(300, 25), wxTE_MULTILINE);
    StatusCtrl->SetFont(tablefont);
    StatusCtrl->SetValue(wxT("连接断开"));

    generateBtn = new wxButton(this, wxID_ANY, wxT("开始传输"), wxPoint(200, 180), wxSize(90, 28), 0);
    generateBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnGenerate, this);
    
    clearBtn = new wxButton(this, wxID_ANY, wxT("清除"), wxPoint(50, 180), wxSize(90, 28), 0);
    clearBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnClear, this);
    
    exitBtn = new wxButton(this, wxID_ANY, wxT("退出"), wxPoint(350, 180), wxSize(90, 28), 0);
    exitBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnQuitserver, this);
    //brouseBtn->Bind(wxEVT_BUTTON, &MyFrame::OnSetFile, this);
    //BrouseBtn = new wxButton(this, wxID_ANY, wxT("浏览"), wxPoint(390, 70), wxSize(85, 25), 0);
    SetBackgroundColour("yellow");


}
//搭建客户端口的框架设计
/*=========================client端口============================*/
ClientFrame::ClientFrame(wxAuiMDIParentFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxAuiMDIChildFrame(parent, id, title, pos, size, style) {
    //BrouseBtn = new wxButton(this, wxID_ANY, wxT("浏览"), wxPoint(390, 70), wxSize(85, 25), 0);
    SetBackgroundColour("green");
    wxFont font(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD, false);
    wxFont tablefont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL, false);
    wxFont tablefont2(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL, false);
    m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("请在下方执行想要的操作:"), wxPoint(120, 5), wxDefaultSize, 0);
    m_staticText1->SetFont(font);
    portxt = new wxStaticText(this, wxID_ANY, wxT(" 当前端口为 :"), wxPoint(10, 35), wxDefaultSize, 0);
    portxt->SetFont(tablefont);
    portbox = new wxListBox(this, wxID_ANY, wxPoint(115, 34), wxSize(90, 25), NULL, 0);
    portbox->SetExtraStyle(wxLB_SINGLE);
    portbox->SetFont(tablefont2);
    portbox->Append("客户端");

    portnumxt = new wxStaticText(this, wxID_ANY, wxT("请输入端口号:"), wxPoint(225, 37), wxDefaultSize, 0);
    portnumxt->SetFont(tablefont);
    portnumberinput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(340, 35), wxSize(133, -1), 0);
    writeTxt = new wxStaticText(this, wxID_ANY, wxT("选择文件夹路径:"), wxPoint(10, 72), wxDefaultSize, 0);
    writeTxt->SetFont(tablefont);
    fileinput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(133, 70), wxSize(252, -1), 0);

    brouseBtn = new wxButton(this, wxID_ANY, wxT("浏览"), wxPoint(390, 70), wxSize(85, 25), 0);
    brouseBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnSetFile, this);
    //bSizer4->Add(m_textCtrl8, 0, wxALL, 5);
    clientipaddress = new wxStaticText(this, wxID_ANY, wxT("请输入本机IP地址:"), wxPoint(10, 113), wxDefaultSize, 0);
    clientipaddress->SetFont(tablefont);
    MytestCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(165, 110), wxSize(220, -1), 0);
    statustxt = new wxStaticText(this, wxID_ANY, wxT("当前状态："), wxPoint(10, 150), wxDefaultSize, 0);
    statustxt->SetFont(tablefont);

    StatusCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(135, 147), wxSize(300, 25), wxTE_MULTILINE);
    StatusCtrl->SetFont(tablefont);
    StatusCtrl->SetValue(wxT("连接断开"));
    generateBtn = new wxButton(this, wxID_ANY, wxT("开始传输"), wxPoint(200, 180), wxSize(90, 28), 0);
    generateBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnGenerate, this);
    clearBtn = new wxButton(this, wxID_ANY, wxT("清除"), wxPoint(50, 180), wxSize(90, 28), 0);
    clearBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnClear, this);
    exitBtn = new wxButton(this, wxID_ANY, wxT("退出"), wxPoint(350, 180), wxSize(90, 28), 0);
    exitBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnQuitclient, this);


}


/*============================事件处理区域=============================*/


// Exit事件句柄
void MyMDIFrame::OnExit(wxCommandEvent& event)
{
    wxMessageDialog dialog(NULL, "确认要退出吗？",
        "Clear Information", wxOK | wxCANCEL | wxICON_INFORMATION);
    if (dialog.ShowModal() == wxID_OK) {
        wxAuiNotebook* notebook = GetNotebook();
        if (notebook)
        {
            notebook->DeleteAllPages();
        }

        event.Skip();
    }

}
// About事件句柄
void MyMDIFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("这是一个发送软件:\n功能：将实现跨平台把指定文件夹的文件发送到另外一个文件夹中去。\n文件的内容有可执行文件、图像、视频、Office 文档，也可能有文本文件等\n使用前请注意一下端口，并在相应的位置填写要求的端口号、IP地址、对应的文件夹等等\n可以按下按钮清除填入的信息",
        "About the file transformimg system", wxOK | wxICON_INFORMATION);
}
void MyMDIFrame::OnClose(wxCloseEvent& evt)
{
    wxMessageDialog dialog(NULL, "确认要退出吗？",
        "Clear Information", wxOK | wxCANCEL | wxICON_INFORMATION);
    if (dialog.ShowModal() == wxID_OK) {
        wxAuiNotebook* notebook = GetNotebook();
        if (notebook)
        {
            notebook->DeleteAllPages();
        }

        evt.Skip();
    }
}

// Hello事件句柄
/*=============================server端口===============================*/
void ServerFrame::OnClear(wxCommandEvent& event)
{
    //wxLogMessage("欢迎使用程序，如果有疑问，可以按快捷键F1进行查找操作流程。");
    wxMessageDialog dialog(NULL, "确认清空所有输入的值？",
        "Clear Information", wxOK | wxCANCEL | wxICON_INFORMATION);
    if (dialog.ShowModal() == wxID_OK) {
        fileinput->SetValue("");
    }

}

void ServerFrame::OnButtonOK(wxCommandEvent& event)
{
    wxString msg;
    msg.Printf("The value is s");
    wxMessageBox(msg, _T("OKButton"), wxOK, this);
}
/*event事件处理，浏览文件夹*/

void ServerFrame::OnSetFile(wxCommandEvent& event)
{
    wxString caption = wxT("Choose a file");

    wxString defaultDir = wxT("/");
    wxString defaultFilename = wxEmptyString;
    wxDirDialog dialog(this, wxT("Testing directory picker"), defaultDir, wxDD_NEW_DIR_BUTTON
    );
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString path = dialog.GetPath();
        //int filterIndex = dialog.GetFilterIndex();
        fileinput->SetValue(path);
    }

}
void ServerFrame::OnQuitserver(wxCommandEvent& event)
{
    wxMessageDialog dialog(NULL, "确认要退出吗？",
        "Clear Information", wxOK | wxCANCEL | wxICON_INFORMATION);
    if (dialog.ShowModal() == wxID_OK) {
        Close(true);
    }

}
void ServerFrame::OnGenerate(wxCommandEvent& event) {
    if (portnumberinput->GetValue() == "")
        wxLogError(wxT("Portnumber empty error...\n端口号不能为空!"));
    else if (fileinput->GetValue() == "")
        wxLogError(wxT("Filename empty error...\n文件路径不能为空!"));
    else if (MytestCtrl->GetValue() == "")
        wxLogError(wxT("Ip address empty error...\n本机IP地址名不能为空!"));
    else {

        host = wxAtoi(portnumberinput->GetValue());
        //端口号：int形式输入
        //addr = "192.168.1.108";
        addr = MytestCtrl->GetValue().mb_str(wxConvUTF8);
        //初始化WSA  
    
#ifdef _WIN32
        WORD sockVersion = MAKEWORD(2, 2);
        WSADATA wsaData;
        WsaStartup(sockVersion, &wsaData);
        SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        ScoketError(slisten);
#elif linux
        int slisten;
        //创建套接字
        slisten = socket(PF_INET, SOCK_STREAM, 0);
        ScoketError(slisten);
#endif

        SetSockoptError(slisten);
        //绑定IP和端口  
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(host);//需要监听的端口

#ifdef _WIN32
        sin.sin_addr.S_un.S_addr = inet_addr(addr);//需要绑定到本地的哪个IP地址
#elif linux
        sin.sin_addr.s_addr = inet_addr(addr);//需要绑定到本地的哪个IP地址
#endif

                                          //进行绑定动作
        Bind(slisten, sin);
        //开始监听  
        Listen(slisten);

        //接收数据  
        struct sockaddr_in remoteAddr;
#ifdef _WIN32
        int nAddrlen = sizeof(remoteAddr);
#elif linux
        socklen_t nAddrlen = sizeof(remoteAddr);
#endif

        char revData[255];
        StatusCtrl->AppendText(wxT("\n正在等待连接......"));
        //statustxt->SetLabelText(wxT("当前状态：正在等待连接..."));
        //printf("等待连接...\n");
        m_Client = accept(slisten, (struct sockaddr*) & remoteAddr, &nAddrlen);//阻塞，直到有新tcp客户端连接

        m_ClientError();
        char scpy[81];
        
        //statustxt->SetLabelText(wxT("当前状态：接收到一个连接："));
        StatusCtrl->AppendText(wxT("\n接收到一个连接："));
        std::ostream stream(StatusCtrl);

        //stream << 123.456 << "some text";
        stream << "\n接收到一个连接：" << inet_ntoa(remoteAddr.sin_addr)<<" 准备传输文件...";;
        stream.flush();
           
        /*=========================================RecvFile()==========================================*/
        programStatus server;
        programStatus client = Start;

        int nlen;
        int filesNum;//文件个数
        int foldersNum;//文件夹个数
        FileHead fh;
        fh.interpos = 0;
        fh.num = 0;
        fh.size = 0;

        isinterrupt(server, client);

        //get numbers（接收发送方传输文件夹数量、文件数量）
        if (client == Start)
        {
            recv(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);
            recv(m_Client, (char*)&filesNum, sizeof(filesNum), 0);
        }
        //在考虑是否删去
        char str[MAX_SIZE] = { 0 };//'yes'
/*===================================需要弹出对话框========================================*/
        wxMessageDialog dialog(NULL, "是否接收文件(yes/no)?",
            "File Confirm Information", wxOK | wxCANCEL | wxICON_INFORMATION);
        //std::cout << "是否接收文件(yes/no)?" << std::endl;
        //std::cin >> str;
        if (dialog.ShowModal() == wxID_OK) {
            strcpy(str,"yes");
        }
        else
            strcpy(str, "no");
        send(m_Client, str, sizeof(str), 0);
        char szPath[MAX_PATH] = { 0 };
        //cout << "请输入存储路径:" << std::endl;
        //cin >> szPath;
        
        strcpy(szPath,(const char*)fileinput->GetValue().mb_str(wxConvUTF8));
        /*================读取文件===============*/
#ifdef _WIN32
        strcat_s(szPath, "\\");
#elif linux
        strcat_s(szPath, "/");
#endif
        getFiles(szPath);
        char severNum[MAX_SIZE];
        sprintf(severNum, "%d.bmp", files.size());

        if (client == Start)
        {
            for (int i = 0; i < foldersNum; i++)
            {
                //获得完整文件夹路径
                char folderPath[MAX_PATH] = { 0 };
                char folderName[MAX_SIZE / 2] = { 0 };
                recv(m_Client, folderName, MAX_SIZE / 2, 0);
                strcpy_s(folderPath, szPath);
                DealSlash(folderName);
                strcat_s(folderPath, folderName);

                //判断文件夹是否存在，不存在则生成文件夹
                if (0 != access(folderPath, 0))
                {
#ifdef _WIN32
                    mkdir(folderPath);
#elif linux
                    strcat_s(folderPath, "/");
                    mkdir(folderPath, 0755);
#endif
                }
            }
        }

        for (int i = 0; i < filesNum; i++)
        {

        file_send:
            int file_flag = 0;//判断有无误码
            int file_count = 0;//用于记录接收文件数量
        inter:
            if (client == Interrupt)
            {
                flag = 0;
                char  szResult[MAX_SIZE] = "yes";
                send(m_Client, (char*)&szResult, sizeof(szResult), 0);

                send(m_Client, (char*)&fh, sizeof(fh), 0);
                recv(m_Client, (char*)&fh, sizeof(fh), 0);//防止fh还未读到就断连 此时fh.size=0,需要让client更新size
                                                          //client = Start;
                                                          //cout << "MD5码" << fh.md5;
            }
            else
            {
                nlen = recv(m_Client, (char*)&fh, sizeof(fh), 0);//获得文件信息
                                                                 //cout << "fh.md5:" << fh.md5;
            }

            if (server == Interrupt)
            {
                flag = 0;
                i = fh.num;
            }
            //client断开连接
            if (nlen == 0 || nlen == -1)
            {
                flag = 0;
                client = Interrupt;
                reconnect();
                isinterrupt(server, client);
                nlen = 1;
                goto inter;
            }

            char szPathName[MAX_PATH] = { 0 };
            DealSlash(fh.str);
            sprintf(szPathName, "%s%s", szPath, fh.str);//拼接路径和文件名
            //cout << szPathName;
            if (access(szPathName, 0) != -1) {
                string szMD5 = FileDigest(szPathName);
                char szmd5[33];
                strcpy_s(szmd5, szMD5.c_str());
                if (strcmp(szmd5, fh.md5) == 0)
                {
                    //cout << "文件已存在！\n";
                    wxLogWarning(wxT("文件已存在!"));
                    send(m_Client, "already", 10, 0);
                    continue;
                }

                else send(m_Client, "not", 10, 0);
            }
            else send(m_Client, "not", 10, 0);

        not_final:
            std::fstream fs;
            if (client == Interrupt)
            {
                fs.open(szPathName, std::fstream::out | std::fstream::binary | std::fstream::app);
                client = Start;
            }
            else if (server == Interrupt)
                fs.open(szPathName, std::fstream::out | std::fstream::binary | std::fstream::in);
            else
                fs.open(szPathName, std::fstream::out | std::fstream::binary | std::fstream::trunc);

            fs.clear();
            fs.seekg(fh.interpos * ONE_PAGE, std::fstream::beg);
            int FileSize = fh.size - fh.interpos * ONE_PAGE;
            int len;
            int k = 0;//记录接收文件及其MD5码个数
            char content[ONE_PAGE] = { 0 };

            while (FileSize)
            {
                if (FileSize < ONE_PAGE)
                    len = recv(m_Client, content, FileSize, 0);
                else
                    len = recv(m_Client, content, ONE_PAGE, 0);

                //client断开连接
                if (len == 0 || len == -1) {

                    wxLogError(wxT("连接已断开，传输失败!"));
                    flag = 0;
                    client = Interrupt;
                    reconnect();
                    isinterrupt(server, client);
                    fs.close();
                    goto inter;
                }

                fs.write(content, len);
                FileSize -= len;
                fh.interpos++;
                if (FileSize == 0) {
                    fh.interpos = 0;
                }
                if (len > 0)
                {
                    char s[10] = "OK";
                    len = send(m_Client, s, 10, 0);
                }
                //没有同步过
                if (flag == 0)
                {
                    isinterrupt(server, client);
                    flag = 1;
                }
            }
            fs.close();
            //cout << "\t接收完成!" << endl;
            wxMessageBox("接收完成!");
            fh.num++;

        }
        wxMessageBox("传输成功,请注意查收!");
        //statustxt->SetLabelText(wxT("当前状态：传输成功，按下按钮继续传输。"));
        StatusCtrl->AppendText(wxT("\n传输成功，按下按钮继续传输。"));
        //cout << "传输成功!请注意查收" << endl;

        /*======================================RecvFile()==============================================*/
#ifdef _WIN32
        closesocket(slisten);//关闭监听socket
        closesocket(m_Client);//关闭socket
        WSACleanup();//卸载

#elif linux
        close(slisten);
        close(m_Client);
#endif
        






    }
}
/*================================下面是Server端口一系列函数=====================================*/
void ServerFrame::ScoketError(SOCKET slisten)
{
#ifdef _WIN32
    if (slisten == INVALID_SOCKET)
    {
        wxLogError(wxT("socket error!"));
        //cout << "socket error !" << endl;
        exit(-1);
    }
#elif linux
    if (slisten < 0)
    {
        wxLogError(wxT("socket error!"));
        exit(-1);
    }
#endif
}
void ServerFrame::SetSockoptError(SOCKET slisten)
{
#ifdef _WIN32
    const char on = 0;
#elif linux
    int on = 1;
#endif

    if (setsockopt(slisten, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
    {
        wxLogError(wxT("Setsockopt failed !"));
        //cout << "setsockopt failed " << endl;
        exit(-1);
    }
}
void ServerFrame::Bind(SOCKET slisten, struct sockaddr_in sin)
{
    if (bind(slisten, (struct sockaddr*) & sin, sizeof(sin)) == -1)//进行绑定动作
    {
        //cout << "bind error !" << endl;
        wxLogError(wxT("A Bind error orrurs !"));
        exit(-1);
    }
}
void ServerFrame::Listen(SOCKET slisten)
{
    if (listen(slisten, 5) == -1)
    {
        //cout << "listen error !" << endl;
        wxLogError(wxT("A Listen error orrurs !"));
        exit(-1);
    }
}
void ServerFrame::m_ClientError()
{
    if (m_Client == -1)
    {
        wxLogError(wxT("An Accept error orrurs !"));
        //cout << "accept error !" << endl;
        exit(-1);
    }
}
void ServerFrame::WsaStartup(WORD sockVersion, WSADATA* wsaData)
{
    if (WSAStartup(sockVersion, wsaData) != 0)
    {
        //cout << "socket初始化失败" << endl;
        wxLogError(wxT("An Accept error orrurs !"));
        return;
    }
}
void ServerFrame::reconnect()
{
#ifdef _WIN32
    closesocket(slisten);//关闭监听socket
    closesocket(m_Client);//关闭socket
    WSACleanup();//卸载

#elif linux
    close(slisten);
    close(m_Client);
#endif

#ifdef _WIN32
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    WsaStartup(sockVersion, &wsaData);
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ScoketError(slisten);
#elif linux
    int slisten;
    //创建套接字
    slisten = socket(PF_INET, SOCK_STREAM, 0);
    ScoketError(slisten);
#endif

    SetSockoptError(slisten);
    //绑定IP和端口  
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(host);//需要监听的端口

#ifdef _WIN32
    sin.sin_addr.S_un.S_addr = inet_addr(addr);//需要绑定到本地的哪个IP地址
#elif linux
    sin.sin_addr.s_addr = inet_addr(addr);//需要绑定到本地的哪个IP地址
#endif

    //进行绑定动作
    Bind(slisten, sin);
    //开始监听  
    Listen(slisten);

    //接收数据  
    struct sockaddr_in remoteAddr;
#ifdef _WIN32
    int nAddrlen = sizeof(remoteAddr);
#elif linux
    socklen_t nAddrlen = sizeof(remoteAddr);
#endif

    char revData[255];
    StatusCtrl->AppendText(wxT("\n正在等待连接..."));
    //statustxt->SetLabelText(wxT("当前状态：正在等待连接..."));
    //printf("等待连接...\n");
    m_Client = accept(slisten, (struct sockaddr*) & remoteAddr, &nAddrlen);//阻塞，直到有新tcp客户端连接

    m_ClientError();
    std::ostream stream(StatusCtrl);

    //stream << 123.456 << "some text";
    stream << "\n重新连接成功！" << inet_ntoa(remoteAddr.sin_addr);
    stream.flush();
    
}
//\和/的互相转换
void ServerFrame::DealSlash(char* path)
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
string ServerFrame::FileDigest(const string& file)
{
    ifstream in(file.c_str(), std::ios::binary);
    if (!in)
        return "";

    MD5 md5;
    std::streamsize length;
    char buffer[MAX_BUFFER];

    while (!in.eof()) {
        in.read(buffer, 1024);
        length = in.gcount();
        if (length > 0)
            md5.update(buffer, length);
    }
    in.close();
    return md5.toString();
}
#ifdef _WIN32
//文件夹路径错误
bool ServerFrame::WindowsPathError(intptr_t* lf, char* folderPath, _finddata_t* file)
{
    if ((*lf = _findfirst(folderPath, file)) == -1) {
        wxLogError(wxT("Filenotfound Error!\n无法找到文件路径！"));
        //cout << "无法找到文件" << endl;
        return true;
    }
    return false;
}
#else linux
void ServerFrame::LinuxPathError(char* path, DIR* dir)
{
    if ((dir = opendir(path)) == NULL)
    {
        wxLogError(wxT("Filenotfound Error!\n无法找到文件路径！"));
        //cout << "无法找到文件" << endl;
    }
}
#endif
bool ServerFrame::getFiles(char* path)
{
    char subFolderPath[MAX_PATH];
    char filePath[MAX_PATH];

#ifdef _WIN32
    char folderPath[MAX_SIZE];
    _finddata_t file;
    intptr_t lf;
    sprintf_s(folderPath, "%s*", path);
    if (WindowsPathError(&lf, folderPath, &file))
        return false;
    else {
        do {
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;

            //是文件夹就递归调用函数
            if (file.attrib & _A_SUBDIR)
            {
                //cout << file.name << endl;
                strcpy_s(subFolderPath, path);
                strcat_s(subFolderPath, file.name);
                strcat_s(subFolderPath, "\\");
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
    LinuxPathErroe(path, dir);
    while ((ptr = readdir(dir)) != NULL)
    {
        memset(filePath, '\0', sizeof(filePath));
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;

        else if (ptr->d_type == 8) //file
        {
            //cout << "filepath:" << path << ptr->d_name << endl;
            strcpy_s(filePath, path);
            strcat_s(filePath, ptr->d_name);
            files.push_back(filePath);
        }
        else if (ptr->d_type == 10)    //link file
        {
            //cout << "filepath:" << path << ptr->d_name << endl;
            strcpy_s(filePath, path);
            strcat_s(filePath, ptr->d_name);
            files.push_back(filePath);
        }
        else if (ptr->d_type == 4)    //文件夹
        {
            memset(subFolderPath, '\0', sizeof(subFolderPath));
            strcpy_s(subFolderPath, path);
            strcat_s(subFolderPath, ptr->d_name);
            strcat_s(subFolderPath, "/");
            getFiles(subFolderPath);
        }
    }
    closedir(dir);
#endif
    return true;
}

void ServerFrame::isinterrupt(programStatus& server, programStatus& client)
{
    recv(m_Client, (char*)&server, sizeof(server), 0);
    send(m_Client, (char*)&client, sizeof(client), 0);
}
/*================================上面是Server端口一系列函数=====================================*/

/*==============================client端口=================================*/
void ClientFrame::OnClear(wxCommandEvent& event)
{
    //wxLogMessage("欢迎使用程序，如果有疑问，可以按快捷键F1进行查找操作流程。");
    wxMessageDialog dialog(NULL, "确认清空所有输入的值？",
        "Clear Information", wxOK | wxCANCEL | wxICON_INFORMATION);
    if (dialog.ShowModal() == wxID_OK) {
        fileinput->SetValue("");
    }

}
void ClientFrame::OnSetFile(wxCommandEvent& event)
{

    // wxString wildcard =wxT("TXT files (*.txt)|*.txt|All files(*.*)|*.*|BMP files (*.bmp)|*.bmp|GIF files (*.gif)|*.gif|ZIP files(*.zip)|*.zip");
    wxString defaultDir = wxT("/");
    wxDirDialog dialog(this, wxT("Please choose a file direction"), defaultDir, wxDD_NEW_DIR_BUTTON
    );
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString path = dialog.GetPath();
        //int filterIndex = dialog.GetFilterIndex();
        fileinput->SetValue(path);
    }

}
void ClientFrame::OnQuitclient(wxCommandEvent& event)
{
    wxMessageDialog dialog(NULL, "确认要退出吗？",
        "Clear Information", wxOK | wxCANCEL | wxICON_INFORMATION);
    if (dialog.ShowModal() == wxID_OK) {
        Close(true);
    }

}
void ClientFrame::OnGenerate(wxCommandEvent& event) {
    if (portnumberinput->GetValue() == "")
        wxLogError(wxT("Portnumber empty error...\n端口号不能为空!"));
    else if (fileinput->GetValue() == "")
        wxLogError(wxT("Filename empty error...\n文件路径不能为空!"));
    else if (MytestCtrl->GetValue() == "")
        wxLogError(wxT("Ip address empty error...\n本机IP地址名不能为空!"));
    else {
        host = wxAtoi(portnumberinput->GetValue());
        //端口号：int形式输入
        //addr = "192.168.1.108";
        addr = MytestCtrl->GetValue().mb_str(wxConvUTF8);

        
#ifdef _WIN32
        WORD sockVersion = MAKEWORD(2, 2);
        WSADATA data;
        if (WSAStartup(sockVersion, &data) != 0)
        {
            wxLogError(wxT("相关的库函数调用失败!"));
        }
#endif
        m_Client = socket(AF_INET, SOCK_STREAM, 0);
        if (m_Client == 0)
        {
            //printf("invalid socket!");
            wxLogError(wxT("invalid socket!"));
            exit(-1);
        }

        struct sockaddr_in serAddr;
        serAddr.sin_family = AF_INET;
        serAddr.sin_port = htons(host);//需要监听的端口
#ifdef _WIN32
        serAddr.sin_addr.S_un.S_addr = inet_addr(addr);//需要绑定到本地的哪个IP地址
#elif linux
        serAddr.sin_addr.s_addr = inet_addr(addr);
#endif

        if (connect(m_Client, (struct sockaddr*) & serAddr, sizeof(serAddr)) == -1)
        {  //连接失败 
            //cout << "连接失败!" << endl;
            wxLogError(wxT("连接失败!\n请检查连接是否有问题。"));
            closeSocket();
            
        }
        else
            StatusCtrl->AppendText(wxT("\n连接成功!准备传输文件!"));
           // statustxt->SetLabelText(wxT("当前状态：连接成功!准备传输文件!"));
            //cout << "连接成功!准备传输文件!\n";
           // wxMessageBox("连接成功!准备传输文件!");
        SendFile();
        closeSocket();

#ifdef _WIN32
        WSACleanup();//卸载
#endif






    }
}
/*================================下面是Client端口一系列函数=====================================*/
void ClientFrame::closeSocket()
{

#ifdef _WIN32
    closesocket(m_Client);//关闭监听socket

#elif linux
    close(m_Client);
#endif
}

void ClientFrame::connectError(sockaddr_in serAddr)
{
    if (connect(m_Client, (struct sockaddr*) & serAddr, sizeof(serAddr)) == -1)
    {  //连接失败 
        wxLogError(wxT("连接失败!\n请检查连接是否有问题。"));
        closeSocket();
        return;
    }
}
void ClientFrame::reconnect()
{
    StatusCtrl->AppendText(wxT("\n服务器连接中断,正尝试重新连接!"));
    //statustxt->SetLabelText(wxT("当前状态：服务器连接中断,正尝试重新连接!"));
    //cout << "\n服务器连接中断!重新连接!" << endl;
    closeSocket();
    m_Client = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(host);//需要监听的端口

#ifdef _WIN32
    serAddr.sin_addr.S_un.S_addr = inet_addr(addr);
    //Sleep(5000);//待机五秒
#elif linux
    serAddr.sin_addr.s_addr = inet_addr(addr);//需要绑定到本地的哪个IP地址
    Sleep(5);//待机五秒
#endif

    connectError(serAddr);
    StatusCtrl->AppendText(wxT("\n服务器连接中断,重新连接成功!"));
    //statustxt->SetLabelText(wxT("当前状态：服务器连接中断,重新连接成功!"));
    //cout << "重新连接成功!";
}
void ClientFrame::isinterrupt(programStatus& server, programStatus& client)
{
    send(m_Client, (char*)&server, sizeof(server), 0);
    recv(m_Client, (char*)&client, sizeof(client), 0);
}
bool ClientFrame::SendFile() {
    FileHead fh;
    programStatus server = Start;
    programStatus client;
    isinterrupt(server, client);

    int rec;
    int equelFlag = 0;

    char path[MAX_PATH] = { 0 };
    //cout << "请输入文件路径:" << endl;
    //cin >> path;
    strcpy(path, (const char*)fileinput->GetValue().mb_str(wxConvUTF8));
    //写入文件
#ifdef _WIN32
    strcat_s(path, "\\");
#elif linux
    strcat_s(path, "/");
#endif
    getFiles(path);
    int pathLen = strlen(path);//文件夹路径长度 格式d:\\xx\\xx\\
	
inter:
    int foldersNum = folders.size();

    int filesNum = files.size();

    if (client == Start)//第一次连接才发送
    {
        send(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);
        send(m_Client, (char*)&filesNum, sizeof(filesNum), 0);
    }
    //如果收到的为yes
    char  szResult[MAX_SIZE] = { 0 };
    recv(m_Client, szResult, sizeof(szResult), 0);
    string severMD5[MAX_SIZE];
    char severNum[MAX_SIZE] = { 0 };
    int severFileNum = 0;
    //如果收到的为yes
    if (0 == strcmp(szResult, "yes"))
    {
        //传输文件夹名称
        //文件夹名/文件夹名
        if (client == Start)
        {
            for (int i = 0; i < foldersNum; i++)
            {
                char* ptemp = const_cast<char*>(folders[i].c_str());//把string转换成char*
                for (int i = 0; i < pathLen; i++)
                    *ptemp++;
                send(m_Client, ptemp, MAX_SIZE / 2, 0);//不想浪费空间
            }
        }
        for (int i = 0; i < filesNum; i++)
        {
            if (server == Interrupt)
                i = fh.num;
            //int file_count_DM5 = i;//本次发送文件对应的DM5数组中的位置就是i
            int is_final = 0;

            /*截取需要的文件路径部分
            文件夹名/文件.txt*/
            char* ptemp = const_cast<char*>(files[i].c_str());//把string转换成char*
            for (int i = 0; i < pathLen; i++)
                *ptemp++;

            char szBuf[ONE_PAGE] = { 0 };

            std::fstream fs;
            if (client == Interrupt)
            {
                recv(m_Client, (char*)&fh, sizeof(fh), 0);
                i = fh.num;
                flag = 0;

                fs.open(files[i], std::fstream::in | std::fstream::binary);
                fs.seekg(0, std::fstream::end);//以最后的位置为基准不偏移
                int nlen = fs.tellg();//取得文件大小
                fs.clear();
                fs.seekg(fh.interpos * ONE_PAGE, std::fstream::beg);
                fh.size = nlen;
                char totalPath[MAX_PATH];
                sprintf(totalPath, "%s%s", path, fh.str);
                string MD5 = FileDigest(totalPath);
                strcpy_s(fh.md5, MD5.c_str());

                char* ptemp = const_cast<char*>(files[i].c_str());//把string转换成char*
                for (int i = 0; i < pathLen; i++)
                    *ptemp++;
                memcpy(fh.str, ptemp, MAX_PATH);
                send(m_Client, (char*)&fh, sizeof(fh), 0);
                //cout << "fh.md5:" << fh.md5 << endl;
                client = Start;
                char already[10];
                recv(m_Client, already, 10, 0);
                if (strcmp(already, "already") == 0)
                {
                    //cout << "文件已存在！";
                    wxLogWarning(wxT("文件已存在！"));
                    continue;
                }
            }
            else if (client == Start && server == Start)
            {
                //发送文件信息
                fs.open(files[i], std::fstream::in | std::fstream::binary);
                fs.seekg(0, std::fstream::end);//以最后的位置为基准不偏移
                int nlen = fs.tellg();//取得文件大小
                fs.clear();
                fs.seekg(0, std::fstream::beg);
                fh.size = nlen;
                memcpy(fh.str, ptemp, MAX_PATH);
                fh.interpos = 0;
                fh.num = i;
                char totalPath[MAX_PATH];
                sprintf(totalPath, "%s%s", path, fh.str);
                string MD5 = FileDigest(totalPath);
                //cout << "MD5:" << MD5 << endl;
                strcpy_s(fh.md5, MD5.c_str());
                //cout << "fh.md5:" << fh.md5 << endl;
                nlen = send(m_Client, (char*)&fh, sizeof(fh), 0);
                std::ostream stream(StatusCtrl);

                //stream << 123.456 << "some text";
                stream << "\n当前文件名：" << files[i].c_str();
                stream.flush();
                
                char already[10];
                recv(m_Client, already, 10, 0);
                if (strcmp(already, "already") == 0)
                {
                    wxLogWarning(wxT("文件已存在！"));
                    //cout << "文件已存在！\n";
                    continue;
                }
            }
            else if (client == Start && server == Interrupt)
            {

                fs.open(files[i], std::fstream::in | std::fstream::binary);
                if (fh.interpos > 0)
                    fh.interpos--;
                send(m_Client, (char*)&fh, sizeof(fh), 0);
                char szBuf[ONE_PAGE] = { 0 };
                //cout << files[i].c_str();

                fs.clear();
                fs.seekg(fh.interpos * ONE_PAGE, std::fstream::beg);
                server = Start;
                char already[10];
                recv(m_Client, already, 10, 0);
                if (strcmp(already, "already") == 0)
                {
                    //cout << "文件已存在！";
                    wxLogWarning(wxT("文件已存在！"));
                    continue;
                }
            }

            //读文件
            while (!fs.eof())
            {
                fs.read(szBuf, ONE_PAGE);
                int len = fs.gcount();
                if (len == 0)
                    return false;
                send(m_Client, szBuf, len, 0);
                fh.interpos++;
                char s[10] = { 0 };
                rec = recv(m_Client, s, 10, 0);
                if (rec == 0 || rec == -1)
                {
                    flag = 0;
                    server = Interrupt;
                    fs.close();
                    reconnect();
                    isinterrupt(server, client);//让服务器明白自己断开过
                    goto inter;
                }
                //断开后传输完终端文件，便恢复断开方的状态为Start
                if (flag == 0)
                {
                    isinterrupt(server, client);
                    flag = 1;
                }
            }
            //关闭文件流
            fs.close();
            StatusCtrl->AppendText(wxT("\n传输完成!"));
            //statustxt->SetLabelText(wxT("当前状态：传输完成!"));
            //cout << "\t传输完成!" << endl;
        }
        //cout << "所有文件传输成功，请注意查收" << endl;
        wxMessageBox(wxT("所有文件传输成功，请注意查收"));
    }
    else
        wxLogWarning(wxT("对方拒绝接收！"));
        //cout << "对方拒绝接收！";

#ifdef _WIN32
    system("pause");
#endif

}
string ClientFrame::FileDigest(const string& file)
{
    ifstream in(file.c_str(), std::ios::binary);
    if (!in)
        return "";

    MD5 md5;
    std::streamsize length;
    char buffer[MAX_BUFFER];
    while (!in.eof()) {
        in.read(buffer, 1024);
        length = in.gcount();
        if (length > 0)
            md5.update(buffer, length);
    }
    in.close();
    return md5.toString();
}
bool ClientFrame::getFiles(char* path)
{

    char subFolderPath[MAX_PATH];
    char filePath[MAX_PATH];

#ifdef _WIN32
    char folderPath[MAX_SIZE];
    _finddata_t file;
    intptr_t lf;
    sprintf_s(folderPath, "%s*", path);
    //输入文件夹路径
    if (WindowsPathError(&lf, folderPath, &file))
        return false;
    else {
        do {
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;
            //是文件夹就递归调用函数
            if (file.attrib & _A_SUBDIR)
            {
                //cout << file.name << endl;
                strcpy_s(subFolderPath, path);
                strcat_s(subFolderPath, file.name);
                folders.push_back(subFolderPath);
                strcat_s(subFolderPath, "\\");
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
    LinuxPathErroe(path, dir);
    while ((ptr = readdir(dir)) != NULL)
    {
        memset(filePath, '\0', sizeof(filePath));
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;
        else if (ptr->d_type == 8) //file
        {
            //cout << "filepath:" << path << ptr->d_name << endl;
            strcpy_s(filePath, path);
            strcat_s(filePath, ptr->d_name);
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
            strcpy_s(subFolderPath, path);
            strcat_s(subFolderPath, ptr->d_name);
            folders.push_back(subFolderPath);
            strcat_s(subFolderPath, "/");
            getFiles(subFolderPath);
        }
    }
    closedir(dir);
#endif
    return true;
}
#ifdef _WIN32
//文件夹路径错误
bool ClientFrame::WindowsPathError(intptr_t* lf, char* folderPath, _finddata_t* file)
{
    if ((*lf = _findfirst(folderPath, file)) == -1) {
        wxLogError(wxT("Filenotfound Error!\n无法找到文件路径！"));
        return true;
    }
    return false;
}
#else linux
void ClientFrame::LinuxPathError(char* path, DIR* dir)
{
    if ((dir = opendir(path)) == NULL)
    {
        wxLogError(wxT("Filenotfound Error!\n无法找到文件路径！"));
    }
}
#endif




/*================================上面是Client端口一系列函数=====================================*/

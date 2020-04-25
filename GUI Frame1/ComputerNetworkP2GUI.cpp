#pragma warning(disable:4996)
#include "md5.h"
#include <wx/wxprec.h>    // Ԥ����ͷ�ļ�
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
/*===================================�ȶ������server����˿�========================================*/
struct FileHead
{
    char str[MAX_PATH] = "";
    int size;
    int num;
    int interpos;
    char md5[33] = "";
};
//����״̬
enum programStatus { Start, Interrupt };
SOCKET m_Client;
int flag = 1;//���ڶϿ����Ӻ�ĵ�һ�θ���״̬
int host;//�˿ں�
const char* addr;//��ַ
SOCKET slisten;

std::vector<string> files;//����ļ�·��
std::vector<string> folders;//����ļ���·��
//string fileMD5[MAX_SIZE];//����ļ���MD5��

/*==================================App����ʹ��======================================*/
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
    wxStaticText* statustxt;//״̬��
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
    wxStaticText* statustxt;//״̬��
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
// �¼���ʶ��
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


/*============================�¼���������=============================*/

// �����¼���
wxBEGIN_EVENT_TABLE(MyMDIFrame, wxAuiMDIParentFrame)
//EVT_MENU(ID_Hello, MyMDIFrame::OnHello)
EVT_MENU(wxID_EXIT, MyMDIFrame::OnExit)
EVT_MENU(wxID_ABOUT, MyMDIFrame::OnAbout)
EVT_CLOSE(OnClose) /// ע�ᴰ�ڹر���Ϣ������


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


// Ӧ�õ����
wxIMPLEMENT_APP(MyApp);


// Ӧ�õ�Init�¼����
bool MyApp::OnInit()
{
    int max = 500;
    MyMDIFrame* frame = new MyMDIFrame("�ļ�����ϵͳ", wxPoint(300, 150), wxSize(530, 350));
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

/*============================����������===============================*/


// ����ܵĳ�ʼ��,�Լ�һϵ�еĲ���
MyMDIFrame::MyMDIFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxAuiMDIParentFrame(NULL, wxID_ANY, title, pos, size)
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Clear, "&�������...\tCtrl-H",
        "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(ID_SetFile, "&��������ļ�...\tCtrl-R", "show value");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "about...\tF1");
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("��ӭʹ�ó�����������ʣ����԰���ݼ�F1���ߵ���˵�����help���в��Ҳ�����");
    SetBackgroundColour("yellow");
    // ����ͼƬ�б�
    // ����ҳ��

}
/*���server�˿ڵĿ��*/
/*==========================server�˿�=============================*/
ServerFrame::ServerFrame(wxAuiMDIParentFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxAuiMDIChildFrame(parent, id, title, pos, size, style) {
    wxFont font(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD, false);
    wxFont tablefont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL, false);
    wxFont tablefont2(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL, false);
    m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("�����·�ִ����Ҫ�Ĳ���:"), wxPoint(120, 5), wxDefaultSize, 0);
    m_staticText1->SetFont(font);
    portxt = new wxStaticText(this, wxID_ANY, wxT(" ��ǰ�˿�Ϊ :"), wxPoint(10, 35), wxDefaultSize, 0);
    portxt->SetFont(tablefont);
    portbox = new wxListBox(this, wxID_ANY, wxPoint(115, 34), wxSize(90, 25), NULL, 0);
    //portbox->SetExtraStyle(wxLB_SINGLE);
    portbox->SetFont(tablefont2);
    portbox->Append("��������");

    portnumxt = new wxStaticText(this, wxID_ANY, wxT("������˿ں�:"), wxPoint(225, 37), wxDefaultSize, 0);
    portnumxt->SetFont(tablefont);
    portnumberinput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(340, 35), wxSize(133, -1), 0);
    writeTxt = new wxStaticText(this, wxID_ANY, wxT("ѡ���ļ���·��:"), wxPoint(10, 72), wxDefaultSize, 0);
    writeTxt->SetFont(tablefont);
    fileinput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(133, 70), wxSize(252, -1), 0);

    brouseBtn = new wxButton(this, wxID_ANY, wxT("���"), wxPoint(390, 70), wxSize(85, 25), 0);
    brouseBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnSetFile, this);
    //bSizer4->Add(m_textCtrl8, 0, wxALL, 5);
    
    serveripaddress = new wxStaticText(this, wxID_ANY, wxT("�����뱾��IP��ַ:"), wxPoint(10, 113), wxDefaultSize, 0);
    serveripaddress->SetFont(tablefont);
   
    MytestCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(165, 110), wxSize(220, -1), 0);
    
    statustxt = new wxStaticText(this, wxID_ANY, wxT("��ǰ״̬��"), wxPoint(10, 150), wxDefaultSize, 0);
    statustxt->SetFont(tablefont);
    StatusCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(135, 147), wxSize(300, 25), wxTE_MULTILINE);
    StatusCtrl->SetFont(tablefont);
    StatusCtrl->SetValue(wxT("���ӶϿ�"));

    generateBtn = new wxButton(this, wxID_ANY, wxT("��ʼ����"), wxPoint(200, 180), wxSize(90, 28), 0);
    generateBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnGenerate, this);
    
    clearBtn = new wxButton(this, wxID_ANY, wxT("���"), wxPoint(50, 180), wxSize(90, 28), 0);
    clearBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnClear, this);
    
    exitBtn = new wxButton(this, wxID_ANY, wxT("�˳�"), wxPoint(350, 180), wxSize(90, 28), 0);
    exitBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnQuitserver, this);
    //brouseBtn->Bind(wxEVT_BUTTON, &MyFrame::OnSetFile, this);
    //BrouseBtn = new wxButton(this, wxID_ANY, wxT("���"), wxPoint(390, 70), wxSize(85, 25), 0);
    SetBackgroundColour("yellow");


}
//��ͻ��˿ڵĿ�����
/*=========================client�˿�============================*/
ClientFrame::ClientFrame(wxAuiMDIParentFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxAuiMDIChildFrame(parent, id, title, pos, size, style) {
    //BrouseBtn = new wxButton(this, wxID_ANY, wxT("���"), wxPoint(390, 70), wxSize(85, 25), 0);
    SetBackgroundColour("green");
    wxFont font(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD, false);
    wxFont tablefont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL, false);
    wxFont tablefont2(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL, false);
    m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("�����·�ִ����Ҫ�Ĳ���:"), wxPoint(120, 5), wxDefaultSize, 0);
    m_staticText1->SetFont(font);
    portxt = new wxStaticText(this, wxID_ANY, wxT(" ��ǰ�˿�Ϊ :"), wxPoint(10, 35), wxDefaultSize, 0);
    portxt->SetFont(tablefont);
    portbox = new wxListBox(this, wxID_ANY, wxPoint(115, 34), wxSize(90, 25), NULL, 0);
    portbox->SetExtraStyle(wxLB_SINGLE);
    portbox->SetFont(tablefont2);
    portbox->Append("�ͻ���");

    portnumxt = new wxStaticText(this, wxID_ANY, wxT("������˿ں�:"), wxPoint(225, 37), wxDefaultSize, 0);
    portnumxt->SetFont(tablefont);
    portnumberinput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(340, 35), wxSize(133, -1), 0);
    writeTxt = new wxStaticText(this, wxID_ANY, wxT("ѡ���ļ���·��:"), wxPoint(10, 72), wxDefaultSize, 0);
    writeTxt->SetFont(tablefont);
    fileinput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(133, 70), wxSize(252, -1), 0);

    brouseBtn = new wxButton(this, wxID_ANY, wxT("���"), wxPoint(390, 70), wxSize(85, 25), 0);
    brouseBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnSetFile, this);
    //bSizer4->Add(m_textCtrl8, 0, wxALL, 5);
    clientipaddress = new wxStaticText(this, wxID_ANY, wxT("�����뱾��IP��ַ:"), wxPoint(10, 113), wxDefaultSize, 0);
    clientipaddress->SetFont(tablefont);
    MytestCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(165, 110), wxSize(220, -1), 0);
    statustxt = new wxStaticText(this, wxID_ANY, wxT("��ǰ״̬��"), wxPoint(10, 150), wxDefaultSize, 0);
    statustxt->SetFont(tablefont);

    StatusCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(135, 147), wxSize(300, 25), wxTE_MULTILINE);
    StatusCtrl->SetFont(tablefont);
    StatusCtrl->SetValue(wxT("���ӶϿ�"));
    generateBtn = new wxButton(this, wxID_ANY, wxT("��ʼ����"), wxPoint(200, 180), wxSize(90, 28), 0);
    generateBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnGenerate, this);
    clearBtn = new wxButton(this, wxID_ANY, wxT("���"), wxPoint(50, 180), wxSize(90, 28), 0);
    clearBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnClear, this);
    exitBtn = new wxButton(this, wxID_ANY, wxT("�˳�"), wxPoint(350, 180), wxSize(90, 28), 0);
    exitBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnQuitclient, this);


}


/*============================�¼���������=============================*/


// Exit�¼����
void MyMDIFrame::OnExit(wxCommandEvent& event)
{
    wxMessageDialog dialog(NULL, "ȷ��Ҫ�˳���",
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
// About�¼����
void MyMDIFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("����һ���������:\n���ܣ���ʵ�ֿ�ƽ̨��ָ���ļ��е��ļ����͵�����һ���ļ�����ȥ��\n�ļ��������п�ִ���ļ���ͼ����Ƶ��Office �ĵ���Ҳ�������ı��ļ���\nʹ��ǰ��ע��һ�¶˿ڣ�������Ӧ��λ����дҪ��Ķ˿ںš�IP��ַ����Ӧ���ļ��еȵ�\n���԰��°�ť����������Ϣ",
        "About the file transformimg system", wxOK | wxICON_INFORMATION);
}
void MyMDIFrame::OnClose(wxCloseEvent& evt)
{
    wxMessageDialog dialog(NULL, "ȷ��Ҫ�˳���",
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

// Hello�¼����
/*=============================server�˿�===============================*/
void ServerFrame::OnClear(wxCommandEvent& event)
{
    //wxLogMessage("��ӭʹ�ó�����������ʣ����԰���ݼ�F1���в��Ҳ������̡�");
    wxMessageDialog dialog(NULL, "ȷ��������������ֵ��",
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
/*event�¼���������ļ���*/

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
    wxMessageDialog dialog(NULL, "ȷ��Ҫ�˳���",
        "Clear Information", wxOK | wxCANCEL | wxICON_INFORMATION);
    if (dialog.ShowModal() == wxID_OK) {
        Close(true);
    }

}
void ServerFrame::OnGenerate(wxCommandEvent& event) {
    if (portnumberinput->GetValue() == "")
        wxLogError(wxT("Portnumber empty error...\n�˿ںŲ���Ϊ��!"));
    else if (fileinput->GetValue() == "")
        wxLogError(wxT("Filename empty error...\n�ļ�·������Ϊ��!"));
    else if (MytestCtrl->GetValue() == "")
        wxLogError(wxT("Ip address empty error...\n����IP��ַ������Ϊ��!"));
    else {

        host = wxAtoi(portnumberinput->GetValue());
        //�˿ںţ�int��ʽ����
        //addr = "192.168.1.108";
        addr = MytestCtrl->GetValue().mb_str(wxConvUTF8);
        //��ʼ��WSA  
    
#ifdef _WIN32
        WORD sockVersion = MAKEWORD(2, 2);
        WSADATA wsaData;
        WsaStartup(sockVersion, &wsaData);
        SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        ScoketError(slisten);
#elif linux
        int slisten;
        //�����׽���
        slisten = socket(PF_INET, SOCK_STREAM, 0);
        ScoketError(slisten);
#endif

        SetSockoptError(slisten);
        //��IP�Ͷ˿�  
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(host);//��Ҫ�����Ķ˿�

#ifdef _WIN32
        sin.sin_addr.S_un.S_addr = inet_addr(addr);//��Ҫ�󶨵����ص��ĸ�IP��ַ
#elif linux
        sin.sin_addr.s_addr = inet_addr(addr);//��Ҫ�󶨵����ص��ĸ�IP��ַ
#endif

                                          //���а󶨶���
        Bind(slisten, sin);
        //��ʼ����  
        Listen(slisten);

        //��������  
        struct sockaddr_in remoteAddr;
#ifdef _WIN32
        int nAddrlen = sizeof(remoteAddr);
#elif linux
        socklen_t nAddrlen = sizeof(remoteAddr);
#endif

        char revData[255];
        StatusCtrl->AppendText(wxT("\n���ڵȴ�����......"));
        //statustxt->SetLabelText(wxT("��ǰ״̬�����ڵȴ�����..."));
        //printf("�ȴ�����...\n");
        m_Client = accept(slisten, (struct sockaddr*) & remoteAddr, &nAddrlen);//������ֱ������tcp�ͻ�������

        m_ClientError();
        char scpy[81];
        
        //statustxt->SetLabelText(wxT("��ǰ״̬�����յ�һ�����ӣ�"));
        StatusCtrl->AppendText(wxT("\n���յ�һ�����ӣ�"));
        std::ostream stream(StatusCtrl);

        //stream << 123.456 << "some text";
        stream << "\n���յ�һ�����ӣ�" << inet_ntoa(remoteAddr.sin_addr)<<" ׼�������ļ�...";;
        stream.flush();
           
        /*=========================================RecvFile()==========================================*/
        programStatus server;
        programStatus client = Start;

        int nlen;
        int filesNum;//�ļ�����
        int foldersNum;//�ļ��и���
        FileHead fh;
        fh.interpos = 0;
        fh.num = 0;
        fh.size = 0;

        isinterrupt(server, client);

        //get numbers�����շ��ͷ������ļ����������ļ�������
        if (client == Start)
        {
            recv(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);
            recv(m_Client, (char*)&filesNum, sizeof(filesNum), 0);
        }
        //�ڿ����Ƿ�ɾȥ
        char str[MAX_SIZE] = { 0 };//'yes'
/*===================================��Ҫ�����Ի���========================================*/
        wxMessageDialog dialog(NULL, "�Ƿ�����ļ�(yes/no)?",
            "File Confirm Information", wxOK | wxCANCEL | wxICON_INFORMATION);
        //std::cout << "�Ƿ�����ļ�(yes/no)?" << std::endl;
        //std::cin >> str;
        if (dialog.ShowModal() == wxID_OK) {
            strcpy(str,"yes");
        }
        else
            strcpy(str, "no");
        send(m_Client, str, sizeof(str), 0);
        char szPath[MAX_PATH] = { 0 };
        //cout << "������洢·��:" << std::endl;
        //cin >> szPath;
        
        strcpy(szPath,(const char*)fileinput->GetValue().mb_str(wxConvUTF8));
        /*================��ȡ�ļ�===============*/
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
                //��������ļ���·��
                char folderPath[MAX_PATH] = { 0 };
                char folderName[MAX_SIZE / 2] = { 0 };
                recv(m_Client, folderName, MAX_SIZE / 2, 0);
                strcpy_s(folderPath, szPath);
                DealSlash(folderName);
                strcat_s(folderPath, folderName);

                //�ж��ļ����Ƿ���ڣ��������������ļ���
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
            int file_flag = 0;//�ж���������
            int file_count = 0;//���ڼ�¼�����ļ�����
        inter:
            if (client == Interrupt)
            {
                flag = 0;
                char  szResult[MAX_SIZE] = "yes";
                send(m_Client, (char*)&szResult, sizeof(szResult), 0);

                send(m_Client, (char*)&fh, sizeof(fh), 0);
                recv(m_Client, (char*)&fh, sizeof(fh), 0);//��ֹfh��δ�����Ͷ��� ��ʱfh.size=0,��Ҫ��client����size
                                                          //client = Start;
                                                          //cout << "MD5��" << fh.md5;
            }
            else
            {
                nlen = recv(m_Client, (char*)&fh, sizeof(fh), 0);//����ļ���Ϣ
                                                                 //cout << "fh.md5:" << fh.md5;
            }

            if (server == Interrupt)
            {
                flag = 0;
                i = fh.num;
            }
            //client�Ͽ�����
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
            sprintf(szPathName, "%s%s", szPath, fh.str);//ƴ��·�����ļ���
            //cout << szPathName;
            if (access(szPathName, 0) != -1) {
                string szMD5 = FileDigest(szPathName);
                char szmd5[33];
                strcpy_s(szmd5, szMD5.c_str());
                if (strcmp(szmd5, fh.md5) == 0)
                {
                    //cout << "�ļ��Ѵ��ڣ�\n";
                    wxLogWarning(wxT("�ļ��Ѵ���!"));
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
            int k = 0;//��¼�����ļ�����MD5�����
            char content[ONE_PAGE] = { 0 };

            while (FileSize)
            {
                if (FileSize < ONE_PAGE)
                    len = recv(m_Client, content, FileSize, 0);
                else
                    len = recv(m_Client, content, ONE_PAGE, 0);

                //client�Ͽ�����
                if (len == 0 || len == -1) {

                    wxLogError(wxT("�����ѶϿ�������ʧ��!"));
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
                //û��ͬ����
                if (flag == 0)
                {
                    isinterrupt(server, client);
                    flag = 1;
                }
            }
            fs.close();
            //cout << "\t�������!" << endl;
            wxMessageBox("�������!");
            fh.num++;

        }
        wxMessageBox("����ɹ�,��ע�����!");
        //statustxt->SetLabelText(wxT("��ǰ״̬������ɹ������°�ť�������䡣"));
        StatusCtrl->AppendText(wxT("\n����ɹ������°�ť�������䡣"));
        //cout << "����ɹ�!��ע�����" << endl;

        /*======================================RecvFile()==============================================*/
#ifdef _WIN32
        closesocket(slisten);//�رռ���socket
        closesocket(m_Client);//�ر�socket
        WSACleanup();//ж��

#elif linux
        close(slisten);
        close(m_Client);
#endif
        






    }
}
/*================================������Server�˿�һϵ�к���=====================================*/
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
    if (bind(slisten, (struct sockaddr*) & sin, sizeof(sin)) == -1)//���а󶨶���
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
        //cout << "socket��ʼ��ʧ��" << endl;
        wxLogError(wxT("An Accept error orrurs !"));
        return;
    }
}
void ServerFrame::reconnect()
{
#ifdef _WIN32
    closesocket(slisten);//�رռ���socket
    closesocket(m_Client);//�ر�socket
    WSACleanup();//ж��

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
    //�����׽���
    slisten = socket(PF_INET, SOCK_STREAM, 0);
    ScoketError(slisten);
#endif

    SetSockoptError(slisten);
    //��IP�Ͷ˿�  
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(host);//��Ҫ�����Ķ˿�

#ifdef _WIN32
    sin.sin_addr.S_un.S_addr = inet_addr(addr);//��Ҫ�󶨵����ص��ĸ�IP��ַ
#elif linux
    sin.sin_addr.s_addr = inet_addr(addr);//��Ҫ�󶨵����ص��ĸ�IP��ַ
#endif

    //���а󶨶���
    Bind(slisten, sin);
    //��ʼ����  
    Listen(slisten);

    //��������  
    struct sockaddr_in remoteAddr;
#ifdef _WIN32
    int nAddrlen = sizeof(remoteAddr);
#elif linux
    socklen_t nAddrlen = sizeof(remoteAddr);
#endif

    char revData[255];
    StatusCtrl->AppendText(wxT("\n���ڵȴ�����..."));
    //statustxt->SetLabelText(wxT("��ǰ״̬�����ڵȴ�����..."));
    //printf("�ȴ�����...\n");
    m_Client = accept(slisten, (struct sockaddr*) & remoteAddr, &nAddrlen);//������ֱ������tcp�ͻ�������

    m_ClientError();
    std::ostream stream(StatusCtrl);

    //stream << 123.456 << "some text";
    stream << "\n�������ӳɹ���" << inet_ntoa(remoteAddr.sin_addr);
    stream.flush();
    
}
//\��/�Ļ���ת��
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
//�ļ���·������
bool ServerFrame::WindowsPathError(intptr_t* lf, char* folderPath, _finddata_t* file)
{
    if ((*lf = _findfirst(folderPath, file)) == -1) {
        wxLogError(wxT("Filenotfound Error!\n�޷��ҵ��ļ�·����"));
        //cout << "�޷��ҵ��ļ�" << endl;
        return true;
    }
    return false;
}
#else linux
void ServerFrame::LinuxPathError(char* path, DIR* dir)
{
    if ((dir = opendir(path)) == NULL)
    {
        wxLogError(wxT("Filenotfound Error!\n�޷��ҵ��ļ�·����"));
        //cout << "�޷��ҵ��ļ�" << endl;
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

            //���ļ��о͵ݹ���ú���
            if (file.attrib & _A_SUBDIR)
            {
                //cout << file.name << endl;
                strcpy_s(subFolderPath, path);
                strcat_s(subFolderPath, file.name);
                strcat_s(subFolderPath, "\\");
                getFiles(subFolderPath);
            }

            //��������·��
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
        else if (ptr->d_type == 4)    //�ļ���
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
/*================================������Server�˿�һϵ�к���=====================================*/

/*==============================client�˿�=================================*/
void ClientFrame::OnClear(wxCommandEvent& event)
{
    //wxLogMessage("��ӭʹ�ó�����������ʣ����԰���ݼ�F1���в��Ҳ������̡�");
    wxMessageDialog dialog(NULL, "ȷ��������������ֵ��",
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
    wxMessageDialog dialog(NULL, "ȷ��Ҫ�˳���",
        "Clear Information", wxOK | wxCANCEL | wxICON_INFORMATION);
    if (dialog.ShowModal() == wxID_OK) {
        Close(true);
    }

}
void ClientFrame::OnGenerate(wxCommandEvent& event) {
    if (portnumberinput->GetValue() == "")
        wxLogError(wxT("Portnumber empty error...\n�˿ںŲ���Ϊ��!"));
    else if (fileinput->GetValue() == "")
        wxLogError(wxT("Filename empty error...\n�ļ�·������Ϊ��!"));
    else if (MytestCtrl->GetValue() == "")
        wxLogError(wxT("Ip address empty error...\n����IP��ַ������Ϊ��!"));
    else {
        host = wxAtoi(portnumberinput->GetValue());
        //�˿ںţ�int��ʽ����
        //addr = "192.168.1.108";
        addr = MytestCtrl->GetValue().mb_str(wxConvUTF8);

        
#ifdef _WIN32
        WORD sockVersion = MAKEWORD(2, 2);
        WSADATA data;
        if (WSAStartup(sockVersion, &data) != 0)
        {
            wxLogError(wxT("��صĿ⺯������ʧ��!"));
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
        serAddr.sin_port = htons(host);//��Ҫ�����Ķ˿�
#ifdef _WIN32
        serAddr.sin_addr.S_un.S_addr = inet_addr(addr);//��Ҫ�󶨵����ص��ĸ�IP��ַ
#elif linux
        serAddr.sin_addr.s_addr = inet_addr(addr);
#endif

        if (connect(m_Client, (struct sockaddr*) & serAddr, sizeof(serAddr)) == -1)
        {  //����ʧ�� 
            //cout << "����ʧ��!" << endl;
            wxLogError(wxT("����ʧ��!\n���������Ƿ������⡣"));
            closeSocket();
            
        }
        else
            StatusCtrl->AppendText(wxT("\n���ӳɹ�!׼�������ļ�!"));
           // statustxt->SetLabelText(wxT("��ǰ״̬�����ӳɹ�!׼�������ļ�!"));
            //cout << "���ӳɹ�!׼�������ļ�!\n";
           // wxMessageBox("���ӳɹ�!׼�������ļ�!");
        SendFile();
        closeSocket();

#ifdef _WIN32
        WSACleanup();//ж��
#endif






    }
}
/*================================������Client�˿�һϵ�к���=====================================*/
void ClientFrame::closeSocket()
{

#ifdef _WIN32
    closesocket(m_Client);//�رռ���socket

#elif linux
    close(m_Client);
#endif
}

void ClientFrame::connectError(sockaddr_in serAddr)
{
    if (connect(m_Client, (struct sockaddr*) & serAddr, sizeof(serAddr)) == -1)
    {  //����ʧ�� 
        wxLogError(wxT("����ʧ��!\n���������Ƿ������⡣"));
        closeSocket();
        return;
    }
}
void ClientFrame::reconnect()
{
    StatusCtrl->AppendText(wxT("\n�����������ж�,��������������!"));
    //statustxt->SetLabelText(wxT("��ǰ״̬�������������ж�,��������������!"));
    //cout << "\n�����������ж�!��������!" << endl;
    closeSocket();
    m_Client = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(host);//��Ҫ�����Ķ˿�

#ifdef _WIN32
    serAddr.sin_addr.S_un.S_addr = inet_addr(addr);
    //Sleep(5000);//��������
#elif linux
    serAddr.sin_addr.s_addr = inet_addr(addr);//��Ҫ�󶨵����ص��ĸ�IP��ַ
    Sleep(5);//��������
#endif

    connectError(serAddr);
    StatusCtrl->AppendText(wxT("\n�����������ж�,�������ӳɹ�!"));
    //statustxt->SetLabelText(wxT("��ǰ״̬�������������ж�,�������ӳɹ�!"));
    //cout << "�������ӳɹ�!";
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
    //cout << "�������ļ�·��:" << endl;
    //cin >> path;
    strcpy(path, (const char*)fileinput->GetValue().mb_str(wxConvUTF8));
    //д���ļ�
#ifdef _WIN32
    strcat_s(path, "\\");
#elif linux
    strcat_s(path, "/");
#endif
    getFiles(path);
    int pathLen = strlen(path);//�ļ���·������ ��ʽd:\\xx\\xx\\
	
inter:
    int foldersNum = folders.size();

    int filesNum = files.size();

    if (client == Start)//��һ�����Ӳŷ���
    {
        send(m_Client, (char*)&foldersNum, sizeof(foldersNum), 0);
        send(m_Client, (char*)&filesNum, sizeof(filesNum), 0);
    }
    //����յ���Ϊyes
    char  szResult[MAX_SIZE] = { 0 };
    recv(m_Client, szResult, sizeof(szResult), 0);
    string severMD5[MAX_SIZE];
    char severNum[MAX_SIZE] = { 0 };
    int severFileNum = 0;
    //����յ���Ϊyes
    if (0 == strcmp(szResult, "yes"))
    {
        //�����ļ�������
        //�ļ�����/�ļ�����
        if (client == Start)
        {
            for (int i = 0; i < foldersNum; i++)
            {
                char* ptemp = const_cast<char*>(folders[i].c_str());//��stringת����char*
                for (int i = 0; i < pathLen; i++)
                    *ptemp++;
                send(m_Client, ptemp, MAX_SIZE / 2, 0);//�����˷ѿռ�
            }
        }
        for (int i = 0; i < filesNum; i++)
        {
            if (server == Interrupt)
                i = fh.num;
            //int file_count_DM5 = i;//���η����ļ���Ӧ��DM5�����е�λ�þ���i
            int is_final = 0;

            /*��ȡ��Ҫ���ļ�·������
            �ļ�����/�ļ�.txt*/
            char* ptemp = const_cast<char*>(files[i].c_str());//��stringת����char*
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
                fs.seekg(0, std::fstream::end);//������λ��Ϊ��׼��ƫ��
                int nlen = fs.tellg();//ȡ���ļ���С
                fs.clear();
                fs.seekg(fh.interpos * ONE_PAGE, std::fstream::beg);
                fh.size = nlen;
                char totalPath[MAX_PATH];
                sprintf(totalPath, "%s%s", path, fh.str);
                string MD5 = FileDigest(totalPath);
                strcpy_s(fh.md5, MD5.c_str());

                char* ptemp = const_cast<char*>(files[i].c_str());//��stringת����char*
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
                    //cout << "�ļ��Ѵ��ڣ�";
                    wxLogWarning(wxT("�ļ��Ѵ��ڣ�"));
                    continue;
                }
            }
            else if (client == Start && server == Start)
            {
                //�����ļ���Ϣ
                fs.open(files[i], std::fstream::in | std::fstream::binary);
                fs.seekg(0, std::fstream::end);//������λ��Ϊ��׼��ƫ��
                int nlen = fs.tellg();//ȡ���ļ���С
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
                stream << "\n��ǰ�ļ�����" << files[i].c_str();
                stream.flush();
                
                char already[10];
                recv(m_Client, already, 10, 0);
                if (strcmp(already, "already") == 0)
                {
                    wxLogWarning(wxT("�ļ��Ѵ��ڣ�"));
                    //cout << "�ļ��Ѵ��ڣ�\n";
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
                    //cout << "�ļ��Ѵ��ڣ�";
                    wxLogWarning(wxT("�ļ��Ѵ��ڣ�"));
                    continue;
                }
            }

            //���ļ�
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
                    isinterrupt(server, client);//�÷����������Լ��Ͽ���
                    goto inter;
                }
                //�Ͽ��������ն��ļ�����ָ��Ͽ�����״̬ΪStart
                if (flag == 0)
                {
                    isinterrupt(server, client);
                    flag = 1;
                }
            }
            //�ر��ļ���
            fs.close();
            StatusCtrl->AppendText(wxT("\n�������!"));
            //statustxt->SetLabelText(wxT("��ǰ״̬���������!"));
            //cout << "\t�������!" << endl;
        }
        //cout << "�����ļ�����ɹ�����ע�����" << endl;
        wxMessageBox(wxT("�����ļ�����ɹ�����ע�����"));
    }
    else
        wxLogWarning(wxT("�Է��ܾ����գ�"));
        //cout << "�Է��ܾ����գ�";

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
    //�����ļ���·��
    if (WindowsPathError(&lf, folderPath, &file))
        return false;
    else {
        do {
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;
            //���ļ��о͵ݹ���ú���
            if (file.attrib & _A_SUBDIR)
            {
                //cout << file.name << endl;
                strcpy_s(subFolderPath, path);
                strcat_s(subFolderPath, file.name);
                folders.push_back(subFolderPath);
                strcat_s(subFolderPath, "\\");
                getFiles(subFolderPath);
            }
            //��������·��
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
        else if (ptr->d_type == 4)    //�ļ���
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
//�ļ���·������
bool ClientFrame::WindowsPathError(intptr_t* lf, char* folderPath, _finddata_t* file)
{
    if ((*lf = _findfirst(folderPath, file)) == -1) {
        wxLogError(wxT("Filenotfound Error!\n�޷��ҵ��ļ�·����"));
        return true;
    }
    return false;
}
#else linux
void ClientFrame::LinuxPathError(char* path, DIR* dir)
{
    if ((dir = opendir(path)) == NULL)
    {
        wxLogError(wxT("Filenotfound Error!\n�޷��ҵ��ļ�·����"));
    }
}
#endif




/*================================������Client�˿�һϵ�к���=====================================*/

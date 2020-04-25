
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
//#include "md5.h"
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

using namespace std;

#define MAX_PATH 260
#define MAX_SIZE 50
#define ONE_PAGE 262144//256*1024

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
class ServerFrame:public wxAuiMDIChildFrame
{
public:
    ServerFrame(wxAuiMDIParentFrame* parent , wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );
    

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
    void OnQuitserver(wxCommandEvent& event);
    void OnButtonOK(wxCommandEvent& event);
    void OnGenerate(wxCommandEvent& event);
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
    void OnQuitclient(wxCommandEvent& event);
    void OnButtonOK(wxCommandEvent& event);
    void OnGenerate(wxCommandEvent& event);
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
    ID_MYCHILD1=4,
    ID_MYCHILD2=5,
    ID_Quitserver=6,
    ID_Quitclient=7
};


/*============================�¼���������=============================*/

// �����¼���
wxBEGIN_EVENT_TABLE(MyMDIFrame, wxAuiMDIParentFrame)
//EVT_MENU(ID_Hello, MyMDIFrame::OnHello)
EVT_MENU(wxID_EXIT, MyMDIFrame::OnExit)
EVT_MENU(wxID_ABOUT, MyMDIFrame::OnAbout)
EVT_CLOSE(OnClose) /// ע�ᴰ�ڹر���Ϣ������


wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ServerFrame,wxAuiMDIChildFrame)
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
    MyMDIFrame *frame = new MyMDIFrame("�ļ�����ϵͳ", wxPoint(300, 150), wxSize(530, 320));
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
    portbox->SetExtraStyle(wxLB_SINGLE);
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

    generateBtn = new wxButton(this, wxID_ANY, wxT("��ʼ����"), wxPoint(200, 150), wxSize(90, 28), 0);
    generateBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnGenerate, this);
    clearBtn = new wxButton(this, wxID_ANY, wxT("���"), wxPoint(50, 150), wxSize(90, 28), 0);
    clearBtn->Bind(wxEVT_BUTTON, &ServerFrame::OnClear, this);
    exitBtn = new wxButton(this, wxID_ANY, wxT("�˳�"), wxPoint(350, 150), wxSize(90, 28), 0);
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

    generateBtn = new wxButton(this, wxID_ANY, wxT("��ʼ����"), wxPoint(200, 150), wxSize(90, 28), 0);
    generateBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnGenerate, this);
    clearBtn = new wxButton(this, wxID_ANY, wxT("���"), wxPoint(50, 150), wxSize(90, 28), 0);
    clearBtn->Bind(wxEVT_BUTTON, &ClientFrame::OnClear, this);
    exitBtn = new wxButton(this, wxID_ANY, wxT("�˳�"), wxPoint(350, 150), wxSize(90, 28), 0);
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
    wxMessageBox("����һ���������:\n���ܣ���ʵ�ֿ�ƽ̨��ָ���ļ��е��ļ����͵�����һ���ļ�����ȥ��\n�ļ��������п�ִ���ļ���ͼ����Ƶ��Office �ĵ���Ҳ�������ı��ļ���",
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
/*==========================server�˿�=============================*/
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

void ServerFrame::OnGenerate(wxCommandEvent& event) {
    if (portnumberinput->GetValue() == "")
        wxLogError(wxT("Portnumber empty error...\n�˿ںŲ���Ϊ��!"));
    else if (fileinput->GetValue() == "")
        wxLogError(wxT("Filename empty error...\n�ļ�·������Ϊ��!"));
    else if (MytestCtrl->GetValue() == "")
        wxLogError(wxT("Ip address empty error...\n����IP��ַ������Ϊ��!"));
    else {


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
    else if(MytestCtrl->GetValue()=="")
        wxLogError(wxT("Ip address empty error...\n����IP��ַ������Ϊ��!"));
    else {
        

    }
}

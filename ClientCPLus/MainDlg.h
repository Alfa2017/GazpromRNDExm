// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

using namespace ATL;

//--[  Dialog ]------------------------------------------
class CMainDlg : public CDialogImpl<CMainDlg>,
                 //public CUpdateUI<CMainDlg>,
                 //public CMessageFilter,
                 //public CIdleHandler,
                 public CWinDataExchange<CMainDlg>,
                 //public CCustomDraw<CMainDlg>,
                 public CDialogResize<CMainDlg>
{
private:
  //WTL::CImageList m_imageList;
  WTL::CString      m_ServerIp;
  WTL::CString      m_ServerPort;
  WTL::CString      m_strMessage;       // <- message

  WSAData data;
  SOCKET Connect;
  SOCKET Listen;

  int _higth;

public:
	enum { IDD = IDD_MAINDLG };

  CMainDlg()
  {
  }

  BEGIN_DDX_MAP(CMainDlg)
    DDX_TEXT(IDC_EDIT_MESSAGE, m_strMessage)
  END_DDX_MAP()

  BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,     OnInitDialog)
    MESSAGE_HANDLER(WM_GETMINMAXINFO,  OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_DESTROY,        OnDestroy)

    //MESSAGE_HANDLER(WM_NOTIFY,     OnNotify)
    //MESSAGE_HANDLER(WM_NCHITTEST,  OnHitTest)
    
    COMMAND_HANDLER(IDC_EDIT_MESSAGE, EN_CHANGE, OnMessageChange)
    COMMAND_ID_HANDLER(IDC_BT_SEND,  OnSendMessage)
		//COMMAND_ID_HANDLER(IDOK,         OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,     OnCancel)   

    CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
    //REFLECT_NOTIFICATIONS()  // <- это нужно(?) для передачи сообщений в карты контролов, но так, как я все обрабатываю в диалоге, то ЗДЕСЬ это не нужно
	END_MSG_MAP()

  BEGIN_DLGRESIZE_MAP(CMainDlg)
  // DLGRESIZE_CONTROL(IDC_VIRTUAL_LIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
  //DLGRESIZE_CONTROL(IDOK,              DLSZ_MOVE_X)
  //DLGRESIZE_CONTROL(IDCANCEL,          DLSZ_MOVE_X)
  //DLGRESIZE_CONTROL(ID_APP_ABOUT,      DLSZ_MOVE_X)
  //DLGRESIZE_CONTROL(IDC_BT_ADD,        DLSZ_MOVE_X)
  //DLGRESIZE_CONTROL(IDC_OWNER_DRAW,    DLSZ_MOVE_Y)
    
  //DLGRESIZE_CONTROL(IDC_EDIT_MESSAGE,  DLSZ_MOVE_Y)
    DLGRESIZE_CONTROL(IDC_EDIT_MESSAGE,  DLSZ_SIZE_X)
    DLGRESIZE_CONTROL(IDC_BT_SEND,       DLSZ_MOVE_X)
  END_DLGRESIZE_MAP()


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
	  // center the dialog on the screen
    ATL::CWindow::CenterWindow();

    // set icons
    HICON hIcon= WTL::AtlLoadIconImage( IDR_MAINFRAME, LR_DEFAULTCOLOR, 32, 32 );
    HICON hIconSmall= WTL::AtlLoadIconImage( IDR_MAINFRAME, LR_DEFAULTCOLOR, 16, 16 );
    SetIcon(hIcon, TRUE);
    SetIcon(hIconSmall, FALSE);

    //// register object for message filtering and idle updates
    //CMessageLoop* pLoop = _Module.GetMessageLoop();
    //ATLASSERT(pLoop != NULL);
    //pLoop->AddMessageFilter(this);

    //UIAddChildWindowContainer(m_hWnd);

    // init resizer
    WTL::CDialogResize<CMainDlg>::DlgResize_Init(); // <-- можно просто DlgResize_Init();

    // se max message length
    CEdit ebMsg = GetDlgItem(IDC_EDIT_MESSAGE);
    ebMsg.SetLimitText(10);

    CRect rect;
    GetWindowRect(&rect);
    _higth = rect.Height();

    m_strMessage = L"Hello!";
    DoDataExchange(DDX_LOAD);

    //load settings
    LoadSettings();

    //sraring up the listener
    WORD version = MAKEWORD(2, 2);
    int res = WSAStartup(version, &data);
    if( res != 0 ) {
      return FALSE;
    }

    struct addrinfo hints;
    struct addrinfo *result;

    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    //getaddrinfo(NULL, "9990", &hints, &result);
    //getaddrinfo("127.0.0.1", "9990", &hints, &result);
    getaddrinfo(CT2A(m_ServerIp), CT2A(m_ServerPort), &hints, &result);
    
    Listen = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    bind(Listen, result->ai_addr, result->ai_addrlen);
    listen(Listen, SOMAXCONN);

    freeaddrinfo(result);

    //AtlTrace(L"OnNotify() -> %x\n", ((LPNMHDR)lParam)->code);
    AtlTrace(L"server ready\n");

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ProcessMessages, (LPVOID)(this), NULL, NULL);

    //char m_connect[] = "connect";
    //while(TRUE) {

    //  if( Connect = accept(Listen, NULL, NULL) ) {
    //    AtlTrace(L"connected\n");
    //    send(Connect, m_connect, strlen(m_connect), NULL);
    //    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendMessageToClient, (LPVOID)(this), NULL, NULL);
    //  }

    //  Sleep(100);
    //}

    //RefreshList();

    return TRUE;

		//// register object for message filtering and idle updates
		//CMessageLoop* pLoop = _Module.GetMessageLoop();
		//ATLASSERT(pLoop != NULL);
		//pLoop->AddMessageFilter(this);
		//pLoop->AddIdleHandler(this);

		//UIAddChildWindowContainer(m_hWnd);

		//return TRUE;
	}

  LRESULT OnGetMinMaxInfo(UINT, WPARAM, LPARAM lParam, BOOL&)
  { 
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;  // load size structure with lParam values

    //lpMMI->ptMinTrackSize.x = 115; // min width
    lpMMI->ptMinTrackSize.y = _higth; // min height
    //lpMMI->ptMaxTrackSize.x = 600; // max width
    lpMMI->ptMaxTrackSize.y = _higth; // max height

    return 0;
  }

  static void ProcessMessages(void *data)
  {
    CMainDlg *dlg = (CMainDlg *)data;

    //char m_connect[] = "connected";
    while (TRUE) {

      if (dlg->Connect = accept(dlg->Listen, NULL, NULL)) {
        AtlTrace(L"connected\n");
        send(dlg->Connect, CT2A(dlg->m_strMessage), dlg->m_strMessage.GetLength(), NULL);
        //send(dlg->Connect, m_connect, strlen(m_connect), NULL);
        ////CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendMessageToClient, (LPVOID)(dlg), NULL, NULL);

        char *buf = new char[1024];
        while (TRUE) {
          int cntRs = 0;
          ZeroMemory(buf, sizeof(buf));
          //memset(buf, 0, sizeof(buf));
          if( (cntRs = recv(dlg->Connect, buf, 1024, NULL)) > 0 ) {
            buf[cntRs] = 0x0;
            dlg->m_strMessage = buf;
            dlg->DoDataExchange(DDX_LOAD);
            AtlTrace(L"rcv=[%s]\n", buf);
          }
          else {
            break;
          }

          Sleep(100);
        }
      }

      Sleep(100);
    }
  }

  //static void SendMessageToClient(void *data)
  //{
  //  CMainDlg *dlg = (CMainDlg *)data;

  //  char *buf = new char[1024];
  //  while(TRUE) {
  //    //ZeroMemory(&hints, sizeof(hints));
  //    memset(buf, 0, sizeof(buf));
  //    if (recv(dlg->Connect, buf, 1024, NULL)) {
  //      AtlTrace(L"[%s]\n", buf);
  //    }

  //    Sleep(100);
  //  }

  //  return;
  //}

  void LoadSettings()
  {
    DWORD cnt = 0;
    WTL::CString server,port;

    server.GetBufferSetLength(60);
    port.GetBufferSetLength(100);

    WTL::CString curDir;
    WTL::CString confFn;
    curDir.GetBufferSetLength(MAX_PATH);
    GetCurrentDirectory(MAX_PATH, curDir.GetBuffer(MAX_PATH));
    //curDir = L"D:\\SGN\\_Development\\Analysis\\Puzzels.GazpromRNDExm\\GazpromRNDExm\\Debug\\";
    confFn.Format(L"%s\\%s", curDir, L"config.ini");

    if( (cnt = GetPrivateProfileString(L"Connection", L"Server", L"127.0.0.1", server.GetBuffer(60), server.GetLength(), confFn)) <= 0) {
      server = L"127.0.0.1";
    }
    if ((cnt = GetPrivateProfileString(L"Connection", L"Port", L"9990", port.GetBuffer(10), port.GetLength(), confFn)) <= 0) {
      port = L"9990";
    }
    m_ServerIp = server;
    m_ServerPort = port;
  }
   
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//// unregister message filtering and idle updates
		//CMessageLoop* pLoop = _Module.GetMessageLoop();
		//ATLASSERT(pLoop != NULL);
		//pLoop->RemoveMessageFilter(this);
		//pLoop->RemoveIdleHandler(this);

		return 0;
	}

  // change the massage
  LRESULT OnMessageChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    DoDataExchange(DDX_SAVE);
    return 0;
  }

  LRESULT OnSendMessage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    //send(Connect, (char *)(LPCTSTR)m_strMessage, m_strMessage.GetLength(), NULL); // only one char....
    send(Connect, CT2A(m_strMessage), m_strMessage.GetLength(), NULL);
    return 0;
  }

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		CloseDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}
};

// TransmitModuleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TransmitModule.h"
#include ".\transmitmoduledlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTransmitModuleDlg 对话框



CTransmitModuleDlg::CTransmitModuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransmitModuleDlg::IDD, pParent)
	, m_enModuleState(MS_INIT)
	, m_iPrintFlag(ILogger::DEFAULT_PRINT_FLAG)
	, m_logLevel(ILogger::DEFAULT_LOG_LEVEL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTransmitModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_START_STOP, m_btnStartStop);
	DDX_Control(pDX, IDC_LOG_LEVEL, m_btnLogLevel);
	DDX_Control(pDX, IDC_PRINT_FLAG, m_btnPrintFlag);
	DDX_Control(pDX, IDC_INFO, m_edtInfo);
	DDX_Control(pDX, IDC_SEND_MAIN, m_btnSendMain);
	DDX_Control(pDX, IDC_SEND_OTHER, m_btnSendOther);
}

BEGIN_MESSAGE_MAP(CTransmitModuleDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START_STOP, OnBnClickedStartStop)
	ON_BN_CLICKED(IDC_LOG_LEVEL, OnBnClickedLogLevel)
	ON_BN_CLICKED(IDC_PRINT_FLAG, OnBnClickedPrintFlag)
	ON_WM_CLOSE()
	//ON_WM_COPYDATA()
	ON_MESSAGE(ILogger::LOG_MESSAGE, OnLoggerMsg)
	ON_BN_CLICKED(IDC_SEND_MAIN, OnBnClickedSendMain)
	ON_BN_CLICKED(IDC_SEND_OTHER, OnBnClickedSendOther)
END_MESSAGE_MAP()

// CTransmitModuleDlg 消息处理程序

BOOL CTransmitModuleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_NORMAL);

	// TODO: 在此添加额外的初始化代码
	
	SetModuleState(MS_INIT);
	UpdateBtnLogLevelText();
	UpdatePrintFlagText();

	srand((UINT)time(NULL));
	
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTransmitModuleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CTransmitModuleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTransmitModuleDlg::UpdateDlgUI()
{
	switch(GetModuleState())
	{
	case MS_INIT:
	case MS_START_ERROR:
	case MS_STOP_OK:
	case MS_STOP_ERROR:
		m_btnStartStop.EnableWindow(TRUE);
		m_btnPrintFlag.EnableWindow(TRUE);
		break;
	case MS_STARTING:
	case MS_STOPING:
		m_btnStartStop.EnableWindow(FALSE);
		m_btnPrintFlag.EnableWindow(FALSE);
		break;
	case MS_START_OK:
		m_btnStartStop.EnableWindow(TRUE);
		m_btnPrintFlag.EnableWindow(FALSE);
		break;
	default:
		;
	}
	m_btnLogLevel.EnableWindow(m_btnPrintFlag.IsWindowEnabled());
}

void CTransmitModuleDlg::OnBnClickedStartStop()
{
	if(!Logger->HasInited())
	{
		// Init
		SetModuleState(MS_STARTING);
		if(Logger->Init(NULL, m_logLevel, m_iPrintFlag))
		{
			SetModuleState(MS_START_OK);
			Logger->SetGUIWindow(GetSafeHwnd());
			Logger->Info(_T("日志对象初始化成功 ^_^"));
		}
		else
		{
			LPCTSTR desc = Logger.ILogger_GetErrorDesc(Logger->GetLastError());
			MessageBox(desc, _T("提示"), MB_OK | MB_ICONEXCLAMATION);
			SetModuleState(MS_START_ERROR);
		}
	}
	else
	{
		// UnInit
		SetModuleState(MS_STOPING);
		if(Logger->UnInit())
		{
			SetModuleState(MS_STOP_OK);
			MessageBox(_T("日志对象关闭成功 ^_^"), _T("提示"), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			SetModuleState(MS_STOP_ERROR);
			MessageBox(_T("日志对象关闭失败 ～～"), _T("提示"), MB_OK | MB_ICONERROR);
		}
	}
	m_btnStartStop.SetWindowText(Logger->HasInited() ? _T("关闭日志") : _T("打开日志"));
}

void CTransmitModuleDlg::OnBnClickedLogLevel()
{
	int level = (m_logLevel + 1);
	if(level > ILogger::LL_FATAL)
		level = level % ILogger::LL_FATAL;
	m_logLevel = (ILogger::LogLevel)(level);
	UpdateBtnLogLevelText();
}

void CTransmitModuleDlg::OnBnClickedPrintFlag()
{
	switch(m_iPrintFlag)
	{
	case 0:
		m_iPrintFlag = ILogger::PRINT_FLAG_FILE;
		break;
	case ILogger::PRINT_FLAG_FILE:
		m_iPrintFlag = ILogger::PRINT_FLAG_SCREEN;;
		break;
	case ILogger::PRINT_FLAG_SCREEN:
		m_iPrintFlag = ILogger::PRINT_FLAG_FILE | ILogger::PRINT_FLAG_SCREEN;
		break;
	case ILogger::PRINT_FLAG_FILE | ILogger::PRINT_FLAG_SCREEN:
		m_iPrintFlag = 0;
		break;
	default:
		m_iPrintFlag = ILogger::PRINT_FLAG_FILE | ILogger::PRINT_FLAG_SCREEN;
	}
	UpdatePrintFlagText();
}

void CTransmitModuleDlg::OnBnClickedSendMain()
{
	// TODO: 在此添加控件通知处理程序代码
	for(int i = 0; i < 1; i++)
	{
		MakeLogInfoFunc(0);
	}
}

void CTransmitModuleDlg::OnBnClickedSendOther()
{
	// TODO: 在此添加控件通知处理程序代码
	for(int i = 0; i < 1; i++)
	{
		AfxBeginThread(MakeLogInfoFunc, (PVOID)1);
	}
}

void CTransmitModuleDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	Logger->SetGUIWindow(NULL);
	CDialog::OnClose();
}

LRESULT CTransmitModuleDlg::OnLoggerMsg(WPARAM wp, LPARAM lp)
{
	// WPARAM： ILoggger 对象指针
	ILogger* pLogger		= (ILogger*)(wp);
	// LPARAM： TLogMsg 对象指针
	ILogger::TLogMsg* plm	= (ILogger::TLogMsg*)(lp);

	SYSTEMTIME& st			= plm->m_stMsgTime;
	
	CString msg;
	msg.Format	(	_T("%02d-%02d %02d:%02d:%02d.%03d\t%6d\t[%-5s]\t%s\r\n"), 
					st.wMonth, st.wDay, st.wHour, 
					st.wMinute, st.wSecond, st.wMilliseconds, 
					plm->m_uiThreadID, 
					Logger.ILogger_GetLogLevelDesc(plm->m_logLevel), plm->m_psMsg
				);

	if(m_edtInfo.GetWindowTextLength() > MAX_INFO_TEXT_LENGT)
		m_edtInfo.SetWindowText(_T(""));

	CString oriMsg;
	m_edtInfo.GetWindowText(oriMsg);
	m_edtInfo.SetWindowText(msg + oriMsg);

	// 销毁 TLogMsg 对象
	Logger->FreeLogMsg(plm);

	return 0;
}

UINT CTransmitModuleDlg::MakeLogInfoFunc(LPVOID  pParam)
{
	if(pParam)
		srand((UINT)::GetCurrentThreadId());

	ILogger::LogLevel ll = (ILogger::LogLevel)(rand() % ILogger::LL_FATAL + 1);

	if(pParam)
		Logger->TryLog(ll, _T("线程 0x%08X 写的日志信息～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～"), ::GetCurrentThreadId());
	else
		Logger->Log_0(ll, _T("主线程写的日志信息◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎"));

	return 0;
}

void CTransmitModuleDlg::UpdateBtnLogLevelText(void)
{
	CString text;
	text.Format(_T("日志级别 [%s]"), Logger.ILogger_GetLogLevelDesc(m_logLevel));
	m_btnLogLevel.SetWindowText(text);
}

void CTransmitModuleDlg::UpdatePrintFlagText(void)
{
	CString pf;
	switch(m_iPrintFlag)
	{
	case 0:
		pf = _T("不打印日志");
		break;
	case ILogger::PRINT_FLAG_FILE:
		pf = _T("只打印到文件");
		break;
	case ILogger::PRINT_FLAG_SCREEN:
		pf = _T("只打印到屏幕");
		break;
	case ILogger::PRINT_FLAG_FILE | ILogger::PRINT_FLAG_SCREEN:
		pf = _T("打印到文件和屏幕");
		break;
	default:
		pf = _T("----------");
	}
	m_btnPrintFlag.SetWindowText(pf);
}

BOOL CTransmitModuleDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (
		pMsg->message == WM_KEYDOWN		
		&&(	pMsg->wParam == VK_ESCAPE	 
		||	pMsg->wParam == VK_CANCEL	
		||	pMsg->wParam == VK_RETURN	
		))
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}

// TransmitModuleDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include <afxmt.h>

#define MAX_INFO_TEXT_LENGT	10240	// 10kB 32768 32KB

// CTransmitModuleDlg 对话框
class CTransmitModuleDlg : public CDialog
{
// 构造
public:
	CTransmitModuleDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TRANSMITMODULE };

	enum ModuleState
	{
		MS_INIT = 0, 
		MS_STARTING, 
		MS_START_OK, 
		MS_START_ERROR,
		MS_STOPING,
		MS_STOP_OK,
		MS_STOP_ERROR
	};

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	ModuleState GetModuleState()
	{
		return m_enModuleState;
	}

	void SetModuleState(ModuleState ms)
	{
		m_enModuleState = ms;
		UpdateDlgUI();
	}

// 实现
protected:
	HICON m_hIcon;

	ModuleState m_enModuleState;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	void UpdateDlgUI();

	static UINT MakeLogInfoFunc(LPVOID  pParam);
	//static UINT LoggerFunc(LPVOID  pParam);

public:
	afx_msg void OnBnClickedStartStop();
	afx_msg void OnBnClickedLogLevel();
	afx_msg void OnBnClickedPrintFlag();
	afx_msg void OnBnClickedSendMain();
	afx_msg void OnBnClickedSendOther();
	afx_msg void OnClose();
	afx_msg LRESULT OnLoggerMsg(WPARAM wp, LPARAM lp);
	//afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	CEdit m_edtInfo;
	CButton m_btnStartStop;
	CButton m_btnLogLevel;
	CButton m_btnPrintFlag;
	CButton m_btnSendMain;
	CButton m_btnSendOther;
private:
	int m_iPrintFlag;
	ILogger::LogLevel m_logLevel;
	void UpdateBtnLogLevelText(void);
	void UpdatePrintFlagText(void);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


	

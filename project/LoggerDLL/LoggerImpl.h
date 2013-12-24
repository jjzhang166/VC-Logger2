/*
 * Copyright Bruce Liang (ldcsaa@gmail.com)
 *
 * Version	: 2.2.1
 * Author	: Bruce Liang
 * Website	: http://www.jessma.org
 * Porject	: https://github.com/ldcsaa
 * Bolg		: http://www.cnblogs.com/ldcsaa
 * WeiBo	: http://weibo.com/u/1402935851
 * QQ Group	: 75375912
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Logger.h"
#include "../Common/Src/STLHelper.h"
#include "../Common/Src/Singleton.h"
#include "../Common/Src/CriticalSection.h"
#include "../Common/Src/Event.h"

/**************************************************/
/**** 是否优化日志记录操作 (负荷不重则不需要优化) ****/

//#define OPTIMIZE_LOGGER

/**************************************************/
/**************** TLogMsg 操作函数 *****************/

// 创建 TLogMsg 对象
ILogger::TLogMsg* TLogMsg_Construct(ILogger::LogLevel ll, UINT tid, LPCTSTR msg, const SYSTEMTIME* pMsgTime = NULL);
// 复制 TLogMsg 对象
ILogger::TLogMsg* TLogMsg_CopyNew(const ILogger::TLogMsg* src);
// 销毁 TLogMsg 对象
void TLogMsg_Destruct(const ILogger::TLogMsg* p);

/**************************************************/
/*********** CLoggerImpl: 日志组件实现类 ***********/

class CLoggerImpl : public ILogger
{
public:
	BOOL Init(
			    LPCTSTR logFile	= NULL
			  , LogLevel ll		= DEFAULT_LOG_LEVEL
			  , int printFlag	= DEFAULT_PRINT_FLAG
			 );
	BOOL UnInit();

public:
	void Log_0(LogLevel ll, LPCTSTR msg);

	BOOL HasInited			()	const {return m_bHasInited;}							
	BOOL IsPrint2File		()	const {return m_iPrintFlag & PRINT_FLAG_FILE;}
	BOOL IsPrint2Screen		()	const {return m_iPrintFlag & PRINT_FLAG_SCREEN;}
	int	 GetPrintFlag		()	const {return m_iPrintFlag;}		
	LogLevel	GetLogLevel	()	const {return m_enLogLevel;}
	LPCTSTR		GetLogFile	()	const {return m_sLogFile;}
	ErrorCode	GetLastError()	const {return m_enLastErrorCode;}

public:
	CLoggerImpl()
	: m_bHasInited			(FALSE)
	, m_iPrintFlag			(0)
	, m_enLogLevel			(LL_NONE)
	, m_hLogThread			(NULL)
	, m_enLastErrorCode		(EC_OK)
	#ifdef _WINDOWS
	, m_hWndGUI				(NULL)
	#endif
	{
	}

	~CLoggerImpl() {if(m_bHasInited) UnInit();}

private:
	inline static LPCTSTR	GetTimeStr(const SYSTEMTIME& st);

#ifdef OPTIMIZE_LOGGER
	inline static LPCTSTR	MakeLogMsg(const TLogMsg* plm);
	inline static LPCSTR	MakeMbcsLogMsg(const TLogMsg* plm);
#else
	inline static CString	MakeLogMsg(const TLogMsg* plm);
	inline static CStringA	MakeMbcsLogMsg(const TLogMsg* plm);
#endif

public:
#ifdef OPTIMIZE_LOGGER
	static const int MAX_LOG_RECORD_LEN	= 4096;	// 单条日志记录最大长度
#endif

private:
	static UINT WINAPI	LogThreadFunc(LPVOID pv);
	static CString		MakeDefaultLogFileName();
	LPCTSTR				GetFullLogFileName();
	void				WaitForWriteLogThreadComplete();

	BOOL				CheckLogFile(LPCTSTR logFile);
	inline ErrorCode	OpenLogFile(HANDLE* hFile);
	inline ErrorCode	OpenLogFile2(HANDLE* hFile);
	static ErrorCode	OSCode2ErrorCode(DWORD osCode);

	void SetLogLevel(LogLevel ll)	{m_enLogLevel = ll;}

	std::list<TLogMsg*>	m_lsLogList;
	
	volatile BOOL		m_bHasInited;
	volatile LogLevel	m_enLogLevel;
	CString				m_sLogFile;
	CString				m_sPath;
	CEvt				m_evLogList;
	CCriSec				m_csLogList;
	int					m_iPrintFlag;
	HANDLE				m_hLogThread;
	ErrorCode			m_enLastErrorCode;

	CString				m_sFullLogFile;
	CTime				m_tLastTime;

/****************************** GUI ******************************/
#ifdef _WINDOWS
	public:
		void SetGUIWindow(HWND hWndGUI)	{m_hWndGUI = hWndGUI;}
		HWND GetGUIWindow()				{return m_hWndGUI;}
	
	private:
		void LogGUIMsg(const TLogMsg* plm);

		volatile HWND m_hWndGUI;

#elif defined(_CONSOLE)
/****************************** Console ******************************/
	private:
		inline void LogConsoleMsg(LPCSTR msg);
#endif

DECLARE_PRIVATE_COPY_CONSTRUCTOR(CLoggerImpl)
};

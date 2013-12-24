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

#include "StdAfx.h"
#include "LoggerImpl.h"

#include <atlconv.h>
#include <process.h>
#include <shlobj.h>

#ifdef _CONSOLE
	#include <iostream>
#endif

#ifdef OPTIMIZE_LOGGER
	#pragma warning(disable: 4996)
#endif

ILogger::TLogMsg* TLogMsg_Construct(ILogger::LogLevel ll, UINT tid, LPCTSTR msg, const SYSTEMTIME* pMsgTime)
{
	size_t size		= sizeof(ILogger::TLogMsg);
	size_t msglen	= 0;

	if(msg)
	{
		msglen	 = (_tcslen(msg) + 1) * sizeof(TCHAR);
		size	+= (msglen - sizeof(TCHAR));
	}

	ILogger::TLogMsg* pLogMsg	= (ILogger::TLogMsg*)malloc((DWORD)size);
	pLogMsg->m_dwSize			= (DWORD)size;
	pLogMsg->m_logLevel			= ll;
	pLogMsg->m_uiThreadID		= tid;

	if(pMsgTime)
		memcpy(&pLogMsg->m_stMsgTime, pMsgTime, sizeof(SYSTEMTIME));
	if(msglen)
		_tcsncpy(pLogMsg->m_psMsg, msg, msglen / (sizeof(TCHAR)));
	else
		pLogMsg->m_psMsg[0] = 0;

	return pLogMsg;
}

ILogger::TLogMsg* TLogMsg_CopyNew(const ILogger::TLogMsg* src)
{
	return TLogMsg_Construct(src->m_logLevel, src->m_uiThreadID, src->m_psMsg, &src->m_stMsgTime);
}

void TLogMsg_Destruct(const ILogger::TLogMsg* p)
{
	free((void*)p);
}

BOOL CLoggerImpl::Init(LPCTSTR logFile, LogLevel ll, int printFlag)
{
	m_enLastErrorCode	= EC_OK;

	if(m_bHasInited)
	{
		m_enLastErrorCode = EC_INVALID_STATE;
		return FALSE;
	}

	if(ll == LL_NONE)
	{
		m_enLastErrorCode = EC_INIT_LOGLEVEL;
		return FALSE;
	}

	if((printFlag & (PRINT_FLAG_FILE | PRINT_FLAG_SCREEN)) == 0)
	{
		m_enLastErrorCode = EC_INIT_PRINTFLAG;
		return FALSE;
	}

	if(printFlag & PRINT_FLAG_FILE)
	{
		CString sLogFile = 
							(logFile == NULL || lstrlen(logFile) == 0) ? 
							MakeDefaultLogFileName() : logFile;

		if(!CheckLogFile(sLogFile))
			return FALSE;
	}

	SetLogLevel(ll);

	m_iPrintFlag = printFlag;
	m_hLogThread = (HANDLE)_beginthreadex(NULL, 0, LogThreadFunc, (LPVOID)this, 0, NULL);

	if(!m_hLogThread)
	{
		m_enLastErrorCode = EC_INIT_CREATE_LOG_THREAD_FAIL;
		return FALSE;
	}

	m_bHasInited = TRUE;

	return TRUE;
}

CString CLoggerImpl::MakeDefaultLogFileName()
{
	CString path;
	::GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();

	int index = path.ReverseFind('\\');
	if(index == -1)
		index = path.ReverseFind('/');

	CString name;
	if(index == -1)
	{
		name = path;
		path = _T(".\\logs\\");
	}
	else
	{
		name = path.Mid(index + 1);
		path = path.Left(index) + _T("\\logs\\");

	}

	index = name.ReverseFind('.');
	if(index == -1)
		name = name + _T(".log");
	else
		name = name.Left(index) + _T(".log");
	return path + name;
}

inline BOOL CLoggerImpl::CheckLogFile(LPCTSTR logFile)
{
	::GetFullPathName(logFile, MAX_PATH, m_sLogFile.GetBuffer(MAX_PATH), NULL);
	m_sLogFile.ReleaseBuffer();

	int index = m_sLogFile.ReverseFind('\\');
	if(index == -1)
		index = m_sLogFile.ReverseFind('/');
	if(index != -1)
		m_sPath = m_sLogFile.Left(index);

	HANDLE hFile;
	ErrorCode result = OpenLogFile2(&hFile);

	if(result != EC_OK)
	{
		m_enLastErrorCode = result;
		return FALSE;
	}

	::CloseHandle(hFile);
	return TRUE;
}

BOOL CLoggerImpl::UnInit()
{
	if(!m_bHasInited)
	{
		m_enLastErrorCode = EC_INVALID_STATE;
		return FALSE;
	}

	SetLogLevel(LL_NONE);

	if(m_hLogThread)
		WaitForWriteLogThreadComplete();

	m_sPath.Empty();
	m_sLogFile.Empty();
	m_sFullLogFile.Empty();

	m_tLastTime		= 0;
	m_iPrintFlag	= DEFAULT_PRINT_FLAG;
	m_bHasInited	= FALSE;

#ifdef _WINDOWS
	SetGUIWindow(NULL);
#endif

	return TRUE;
}

void CLoggerImpl::WaitForWriteLogThreadComplete()
{
	m_evLogList.Set();
	VERIFY(::WaitForSingleObject(m_hLogThread, INFINITE) == WAIT_OBJECT_0);
	
	::CloseHandle(m_hLogThread);
	m_hLogThread	= NULL;
}

inline CLoggerImpl::ErrorCode CLoggerImpl::OpenLogFile(HANDLE* hFile)
{
	*hFile = ::CreateFile(	GetFullLogFileName(), 
							GENERIC_READ | GENERIC_WRITE, 
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
							NULL, 
							OPEN_ALWAYS, 
							FILE_ATTRIBUTE_NORMAL,
							NULL);

	if(*hFile == INVALID_HANDLE_VALUE)
		return OSCode2ErrorCode(::GetLastError());

	return EC_OK;
}

inline CLoggerImpl::ErrorCode CLoggerImpl::OpenLogFile2(HANDLE* hFile)
{
	ErrorCode ec = OpenLogFile(hFile);

	if(ec == EC_FILE_BADPATH)
	{
		if(!::PathFileExists(m_sPath))
			::SHCreateDirectoryEx(NULL, m_sPath, NULL);

		ec = OpenLogFile(hFile);
	}

	return ec;
}

LPCTSTR CLoggerImpl::GetFullLogFileName()
{
	CTime curTime = CTime::GetCurrentTime();

	if	(
			m_sFullLogFile.IsEmpty()						||
			curTime.GetDay()	!= m_tLastTime.GetDay()		||
			curTime.GetMonth()	!= m_tLastTime.GetMonth()	||
			curTime.GetYear()	!= m_tLastTime.GetYear()
		)
	{
		m_tLastTime		= curTime;
		m_sFullLogFile	= m_sLogFile;
		CString strDate	= curTime.Format(_T("-%Y%m%d")); 
		int index		= m_sLogFile.ReverseFind('.');
		index	  != -1 ? m_sFullLogFile.Insert(index, strDate) : m_sFullLogFile.Append(strDate);
	}

	return m_sFullLogFile;
}

void CLoggerImpl::Log_0(LogLevel ll, LPCTSTR msg)
{
	if(!m_bHasInited)
	{
		m_enLastErrorCode = EC_INVALID_STATE;
		return;
	}

	if(ll < GetLogLevel())
		return;

	TLogMsg* plm = TLogMsg_Construct(ll, ::GetCurrentThreadId(), msg);

	{
		CCriSecLock locallock(m_csLogList);

		if(ll < GetLogLevel())
		{
			TLogMsg_Destruct(plm);
			return;
		}

		SYSTEMTIME st;
		::GetLocalTime(&st);
		memcpy(&plm->m_stMsgTime, &st, sizeof(SYSTEMTIME));
		m_lsLogList.push_back(plm);
	}

	m_evLogList.Set();
}

UINT WINAPI CLoggerImpl::LogThreadFunc(LPVOID pv)
{
	CLoggerImpl* pThis	= (CLoggerImpl*)pv;
	ASSERT(pThis);

	list<TLogMsg*> lsLogList;

	while(TRUE)
	{
		VERIFY(::WaitForSingleObject(pThis->m_evLogList, INFINITE) == WAIT_OBJECT_0);
		
		{
			CCriSecLock locallock(pThis->m_csLogList);

			if(pThis->GetLogLevel() != LL_NONE)
				pThis->m_lsLogList.swap(lsLogList);
			else
			{
				while(pThis->m_lsLogList.size() > 0)
				{
					TLogMsg_Destruct(pThis->m_lsLogList.front());
					pThis->m_lsLogList.pop_front();
				}

				return 0;
			}
		}

		BOOL isPrintToFile		= pThis->IsPrint2File();
		BOOL isPrintToScreen	= pThis->IsPrint2Screen();

		HANDLE hFile	= INVALID_HANDLE_VALUE;
		DWORD writeLen	= 0;

		if(isPrintToFile)
		{
			if(pThis->OpenLogFile2(&hFile) == EC_OK)
				::SetFilePointer(hFile, 0, NULL, FILE_END);
			else
				isPrintToFile = FALSE;
		}

		while(lsLogList.size() > 0)
		{
			TLogMsg* plm = lsLogList.front();

#ifdef _WINDOWS
			if(isPrintToScreen)
				pThis->LogGUIMsg(plm);

			if(isPrintToFile)
			{
#ifdef OPTIMIZE_LOGGER
				LPCSTR
#else
				CStringA
#endif
					msg = pThis->MakeMbcsLogMsg(plm);

				::WriteFile(hFile, msg, (DWORD)strlen(msg), &writeLen, NULL);
			}
#elif defined (_CONSOLE)
#ifdef OPTIMIZE_LOGGER
			LPCSTR
#else
			CStringA
#endif
					msg = pThis->MakeMbcsLogMsg(plm);

			if(isPrintToScreen)
				pThis->LogConsoleMsg(msg);

			if(isPrintToFile)
				::WriteFile(hFile, msg, (DWORD)strlen(msg), &writeLen, NULL);
#endif
			TLogMsg_Destruct(plm);
			lsLogList.pop_front();
		}

		if(isPrintToFile)
			::CloseHandle(hFile);
	}

#ifdef _WIN64
	#define SSS 1
#elif defined(_WIN32)
	#define SSS 2
#endif
	return 0;
}

#ifdef _WINDOWS

void CLoggerImpl::LogGUIMsg(const TLogMsg* plm)
{
	if(m_hWndGUI)
	{
		TLogMsg* pMsg = TLogMsg_CopyNew(plm);
		if(!::PostMessage(m_hWndGUI, LOG_MESSAGE, LOGGER_FAKE_WINDOW_BASE | (WPARAM)this, (LPARAM)pMsg))
			TLogMsg_Destruct(pMsg);
	}
}

#elif defined(_CONSOLE)

inline void CLoggerImpl::LogConsoleMsg(LPCSTR msg)
{
	// log to console not need to SYNC

	std::cout << msg;
}

#endif

inline LPCTSTR CLoggerImpl::GetTimeStr(const SYSTEMTIME& st)
{
	static TCHAR t[24];

	_stprintf(	t, 
				_T("%02d:%02d:%02d.%03d"), 
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return t;
}

#ifdef OPTIMIZE_LOGGER

inline LPCTSTR CLoggerImpl::MakeLogMsg(const TLogMsg* plm)
{
	static TCHAR msg[MAX_LOG_RECORD_LEN];

	_stprintf(	msg, _T("%s %6d  [%-5s]  %s\r\n"), 
				GetTimeStr(plm->m_stMsgTime), 
				plm->m_uiThreadID, 
				ILogger_GetLogLevelDesc(plm->m_logLevel), plm->m_psMsg);
	return msg;
}

inline LPCSTR CLoggerImpl::MakeMbcsLogMsg(const TLogMsg* plm)
{
	LPCTSTR str	= MakeLogMsg(plm);

#ifdef _UNICODE
	USES_CONVERSION;

	static char msg[MAX_LOG_RECORD_LEN];
	const char* pstr = T2A((LPTSTR)(LPCTSTR)str);
	memmove(msg, pstr, strlen(pstr) + 1);
	return msg;
#else
	return str;
#endif
}

#else	// undef OPTIMIZE_LOGGER

inline CString CLoggerImpl::MakeLogMsg(const TLogMsg* plm)
{
	CString str; 
	str.Format( _T("%s %6d  [%-5s]  %s\r\n"), 
				GetTimeStr(plm->m_stMsgTime), 
				plm->m_uiThreadID, 
				ILogger_GetLogLevelDesc(plm->m_logLevel), plm->m_psMsg);
	return str;
}

inline CStringA CLoggerImpl::MakeMbcsLogMsg(const TLogMsg* plm)
{
#ifdef _UNICODE
	USES_CONVERSION;

	CString str			= MakeLogMsg(plm);
	const char* pstr	= T2A((LPTSTR)(LPCTSTR)str);
	return pstr;
#else	// undef _UNICODE
	return MakeLogMsg(plm);
#endif	// _UNICODE
}

#endif	// OPTIMIZE_LOGGER

CLoggerImpl::ErrorCode CLoggerImpl::OSCode2ErrorCode(DWORD osCode)
{
	switch (osCode)
	{
	case NO_ERROR:
		return CLoggerImpl::EC_OK;
	case ERROR_FILE_NOT_FOUND:
	case ERROR_NO_MORE_FILES:
	case ERROR_INVALID_HANDLE:
	case ERROR_DISK_CHANGE:
		return CLoggerImpl::EC_FILE_FILENOTFOUND;
	case ERROR_PATH_NOT_FOUND:
	case ERROR_LABEL_TOO_LONG:
	case ERROR_FILENAME_EXCED_RANGE:
	case ERROR_META_EXPANSION_TOO_LONG:
	case ERROR_DIRECTORY:
	case ERROR_BAD_PATHNAME:
	case ERROR_INVALID_NAME:
	case ERROR_DIR_NOT_ROOT:
	case ERROR_INVALID_LEVEL:
	case ERROR_NO_VOLUME_LABEL:
	case ERROR_BUFFER_OVERFLOW:
	case ERROR_ALREADY_ASSIGNED:
	case ERROR_SHARING_PAUSED:
	case ERROR_DUP_NAME:
	case ERROR_DEV_NOT_EXIST:
	case ERROR_BAD_NETPATH:
	case ERROR_BAD_NET_NAME:
	case ERROR_NOT_SAME_DEVICE:
	case ERROR_WRONG_DISK:
	case ERROR_INVALID_DRIVE:
		return CLoggerImpl::EC_FILE_BADPATH;
	case ERROR_TOO_MANY_OPEN_FILES:
	case ERROR_NO_MORE_SEARCH_HANDLES:
	case ERROR_TOO_MANY_NAMES:
	case ERROR_SHARING_BUFFER_EXCEEDED:
		return CLoggerImpl::EC_FILE_TOMANYOPERFILES;
	case ERROR_ACCESS_DENIED:
	case ERROR_WRITE_PROTECT:
	case ERROR_WRITE_FAULT:
	case ERROR_BUSY:
	case ERROR_REQ_NOT_ACCEP:
	case ERROR_SWAPERROR:
	case ERROR_ALREADY_EXISTS:
	case ERROR_FILE_EXISTS:
	case ERROR_CANNOT_MAKE:
	case ERROR_INVALID_PASSWORD:
	case ERROR_NETNAME_DELETED:
	case ERROR_NETWORK_ACCESS_DENIED:
	case ERROR_BAD_NET_RESP:
	case ERROR_NETWORK_BUSY:
	case ERROR_INVALID_ACCESS:
		return CLoggerImpl::EC_FILE_ACCESSDENIED;
	case ERROR_BAD_FORMAT:
	case ERROR_INVALID_ORDINAL:
	case ERROR_INVALID_EXE_SIGNATURE:
	case ERROR_BAD_EXE_FORMAT:
	case ERROR_INVALID_TARGET_HANDLE:
	case ERROR_BAD_DEV_TYPE:
	case ERROR_BAD_REM_ADAP:
	case ERROR_NOT_DOS_DISK:
		return CLoggerImpl::EC_FILE_INVALIDFILE;
	case ERROR_CURRENT_DIRECTORY:
	case ERROR_DIR_NOT_EMPTY:
		return CLoggerImpl::EC_FILE_REMOVECURRENTDIR;
	case ERROR_NO_SPOOL_SPACE:
		return CLoggerImpl::EC_FILE_DIRECTORYFULL;
	case ERROR_NEGATIVE_SEEK:
	case ERROR_SEEK_ON_DEVICE:
	case ERROR_BAD_LENGTH:
	case ERROR_SEEK:
	case ERROR_SECTOR_NOT_FOUND:
	case ERROR_READ_FAULT:
		return CLoggerImpl::EC_FILE_BADSEEK;
	case ERROR_BAD_UNIT:
	case ERROR_NOT_READY:
	case ERROR_BAD_COMMAND:
	case ERROR_CRC:
	case ERROR_ADAP_HDW_ERR:
	case ERROR_UNEXP_NET_ERR:
	case ERROR_NET_WRITE_FAULT:
	case ERROR_INVALID_CATEGORY:
	case ERROR_OPERATION_ABORTED:
	case ERROR_IO_INCOMPLETE:
	case ERROR_IO_PENDING:
		return CLoggerImpl::EC_FILE_HARDIO;
	case ERROR_SHARING_VIOLATION:
		return CLoggerImpl::EC_FILE_SHARINGVIOLATION;
	case ERROR_LOCK_VIOLATION:
	case ERROR_LOCK_FAILED:
	case ERROR_DRIVE_LOCKED:
		return CLoggerImpl::EC_FILE_LOCKVIOLATION;
	case ERROR_HANDLE_DISK_FULL:
	case ERROR_DISK_FULL:
		return CLoggerImpl::EC_FILE_DISKFULL;
	case ERROR_HANDLE_EOF:
		return CLoggerImpl::EC_FILE_ENDOFFILE;
	default:
		return CLoggerImpl::EC_FILE_GENERIC;
	}
}

/*
 * Copyright Bruce Liang (ldcsaa@gmail.com)
 *
 * Version	: 2.0.3
 * Author	: Bruce Liang
 * Website	: http://www.jessma.org
 * Project	: https://github.com/ldcsaa
 * Blog		: http://www.cnblogs.com/ldcsaa
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
#include "StaticLogger.h"
#include "LoggerImpl.h"

EXTERN_C LOGGER_API ILogger* ILogger_Create()
{
	return new CLoggerImpl;
}

EXTERN_C LOGGER_API void ILogger_Destroy(ILogger* p)
{
	delete p;
}

EXTERN_C LOGGER_API LPCTSTR ILogger_GetLogLevelDesc(ILogger::LogLevel ll)
{
	switch(ll)
	{
	case ILogger::LL_NONE:
		return _T("NONE");
	case ILogger::LL_DEBUG:
		return _T("DEBUG");
	case ILogger::LL_TRACE:
		return _T("TRACE");
	case ILogger::LL_INFO:
		return _T("INFO");
	case ILogger::LL_WARN:
		return _T("WARN");
	case ILogger::LL_ERROR:
		return _T("ERROR");
	case ILogger::LL_FATAL:
		return _T("FATAL");
	default:
		return _T("-----");
	}
}

EXTERN_C LOGGER_API LPCTSTR ILogger_GetErrorDesc(ILogger::ErrorCode ec)
{
	switch(ec)
	{
	case ILogger::EC_OK:
		return _T("No error occurred");
	case ILogger::EC_FILE_GENERIC:
		return _T("打开日志文件异常 -> An unspecified error occurred");
	case ILogger::EC_FILE_FILENOTFOUND:
		return _T("打开日志文件异常 -> The file could not be located");
	case ILogger::EC_FILE_BADPATH:
		return _T("打开日志文件异常 -> All or part of the path is invalid");
	case ILogger::EC_FILE_TOMANYOPERFILES:
		return _T("打开日志文件异常 -> The permitted number of open files was exceeded");
	case ILogger::EC_FILE_ACCESSDENIED:
		return _T("打开日志文件异常 -> The file could not be accessed");
	case ILogger::EC_FILE_INVALIDFILE:
		return _T("打开日志文件异常 -> There was an attempt to use an invalid file handle");
	case ILogger::EC_FILE_REMOVECURRENTDIR:
		return _T("打开日志文件异常 -> The current working directory cannot be removed");
	case ILogger::EC_FILE_DIRECTORYFULL:
		return _T("打开日志文件异常 -> There are no more directory entries");
	case ILogger::EC_FILE_BADSEEK:
		return _T("打开日志文件异常 -> There was an error trying to set the file pointer");
	case ILogger::EC_FILE_HARDIO:
		return _T("打开日志文件异常 -> There was a hardware error");
	case ILogger::EC_FILE_SHARINGVIOLATION:
		return _T("打开日志文件异常 -> SHARE.EXE was not loaded, or a shared region was locked");
	case ILogger::EC_FILE_LOCKVIOLATION:
		return _T("打开日志文件异常 -> There was an attempt to lock a region that was already locked");
	case ILogger::EC_FILE_DISKFULL:
		return _T("打开日志文件异常 -> The disk is full");
	case ILogger::EC_FILE_ENDOFFILE:
		return _T("打开日志文件异常 -> The end of file was reached");
	case ILogger::EC_INVALID_STATE:
		return _T("非法调用 -> 不能在组件的当前状态执行本调用");
	case ILogger::EC_INIT_LOGLEVEL:
		return _T("参数错误 (ll) -> 日志级别设置错误 (不能为'LL_NONE')");
	case ILogger::EC_INIT_PRINTFLAG:
		return _T("参数错误 (printFlag) -> 至少要把日志输出到一个载体");
	case ILogger::EC_INIT_CREATE_LOG_THREAD_FAIL:
		return _T("创建日志线程失败");
	default:
		return _T("未知错误");
	}
}

ILogger::ILogger(void) {}
ILogger::~ILogger(void) {}

void ILogger::Debug_0	(LPCTSTR msg) {Log_0(LL_DEBUG,	msg);}
void ILogger::Trace_0	(LPCTSTR msg) {Log_0(LL_TRACE,	msg);}
void ILogger::Info_0	(LPCTSTR msg) {Log_0(LL_INFO,	msg);}
void ILogger::Warn_0	(LPCTSTR msg) {Log_0(LL_WARN,	msg);}
void ILogger::Error_0	(LPCTSTR msg) {Log_0(LL_ERROR,	msg);}
void ILogger::Fatal_0	(LPCTSTR msg) {Log_0(LL_FATAL,	msg);}

void ILogger::LogV(LogLevel ll, LPCTSTR format, va_list arg_ptr)
{
	CString str;
	str.FormatV(format, arg_ptr);
	Log_0(ll, str);
}

void ILogger::Log(LogLevel ll, LPCTSTR format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(ll, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::Debug(LPCTSTR format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_DEBUG, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::Trace(LPCTSTR format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_TRACE, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::Info(LPCTSTR format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_INFO, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::Warn(LPCTSTR format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_WARN, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::Error(LPCTSTR format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_ERROR, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::Fatal(LPCTSTR format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_FATAL, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::TryLog(LogLevel ll, LPCTSTR format, ...)
{
	if(ll < GetLogLevel())
		return;

	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(ll, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::TryDebug(LPCTSTR format, ...)
{
	if(LL_DEBUG < GetLogLevel())
		return;

	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_DEBUG, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::TryTrace(LPCTSTR format, ...)
{
	if(LL_TRACE < GetLogLevel())
		return;

	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_TRACE, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::TryInfo(LPCTSTR format, ...)
{
	if(LL_INFO < GetLogLevel())
		return;

	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_INFO, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::TryWarn(LPCTSTR format, ...)
{
	if(LL_WARN < GetLogLevel())
		return;

	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_WARN, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::TryError(LPCTSTR format, ...)
{
	if(LL_ERROR < GetLogLevel())
		return;

	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_ERROR, format, arg_ptr);
	va_end(arg_ptr);
}

void ILogger::TryFatal(LPCTSTR format, ...)
{
	if(LL_FATAL < GetLogLevel())
		return;

	va_list arg_ptr;
	va_start(arg_ptr, format);
	LogV(LL_FATAL, format, arg_ptr);
	va_end(arg_ptr);
}

#ifdef _WINDOWS

void ILogger::FreeLogMsg(const TLogMsg* pLogMsg)
{
	::TLogMsg_Destruct(pLogMsg);
}

#endif

CStaticLogger::CStaticLogger(BOOL bCreate)
{
	if(bCreate)
		m_pLogger = ::ILogger_Create();
	else
		m_pLogger = NULL;
}

CStaticLogger::~CStaticLogger()
{
	Reset(NULL);
}

void CStaticLogger::Reset(ILogger* pLogger)
{
	if(m_pLogger)
		::ILogger_Destroy(m_pLogger);

	m_pLogger = pLogger;
}

inline BOOL CStaticLogger::IsValid			()	const	{return m_pLogger != NULL;}
inline ILogger* CStaticLogger::Get			()	const	{return m_pLogger;}
inline ILogger& CStaticLogger::operator *	()	const	{return *m_pLogger;}
inline ILogger* CStaticLogger::operator ->	()	const	{return m_pLogger;}
inline CStaticLogger::operator ILogger*		()	const	{return m_pLogger;}
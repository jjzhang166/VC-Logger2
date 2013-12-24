
（注：VC-Logger v2.0.3 依赖 vc-common-src-2.3.2 中的基础公共代码。因此，编译 VC-Logger v2.0.3 时需要同时下载 vc-common-src-2.2.2）

/******************************************************************************
Module:  VC-Logger

Purpose: 记录程序日志。
		1. 把日志信息输出到指定文件
		2. 对于 GUI 程序，可以把日志信息发送到指定窗口
		3. 对于Console应用程序，可以把日志信息发往标准输出 (std::cout)

Desc:
		1、功能：
		--------------------------------------------------------------------------------------
		a) 把日志信息输出到指定文件
		b) 每日生成一个日志文件
		c) 对于 GUI 程序，可以把日志信息发送到指定窗口
		d) 对于Console应用程序，可以把日志信息发往标准输出 (std::cout)
		e) 支持 MBCS / UNICODE，Console / GUI，win32 / x64 程序
		f) 支持动态加载和静态加载日志组件 DLL
		g) 支持 DEBUG/TRACE/INFO/WARN/ERROR/FATAL 等多个日志级别
		
		2、可用性：
		--------------------------------------------------------------------------------------
		a) 简单纯净：不依赖任何程序库或框架
		b) 使用接口简单，不需复杂的配置或设置工作
		c) 提供 CStaticLogger 和 CDynamicLogger 包装类用于静态或动态加载以及操作日志组件，用户无需关注加载细节
		d) 程序如果要记录多个日志文件只需为每个日志文件创建相应的 CStaticLogger 或 CDynamicLogger 对象
		e) 只需调用 Log()/Debug()/Trace()/Info()/Warn()/Error()/Fatal() 等方法记录日志
		f) 日志记录方法支持可变参数
		g) 日志输出格式：<时间> <线程ID> <日志级别> <日志内容>
		
		3、性能：
		--------------------------------------------------------------------------------------
		a) 支持多线程同时发送写日志请求
		b) 使用单独线程在后台写日志，不影响工作线程的正常执行
		c) 采用批处理方式批量记录日志

Usage:
		方法一：（静态加载 Logger DLL）
		--------------------------------------------------------------------------------------
		0. 应用程序包含 StaticLogger.h 头文件
		1. 创建 CStaticLogger 对象（通常为全局对象）
		2. 调用 CStaticLogger->Init(...) 初始化日志组件
		3. 使用 CStaticLogger->Log()/Debug()/Trace()/Info()/Warn()/Error()/Fatal() 等方法写日志
		4. 调用 CStaticLogger->UnInit(...) 清理日志组件（CStaticLogger 对象析构时也会自动清理日志组件）

		方法二：（动态加载 Logger DLL）
		--------------------------------------------------------------------------------------
		0. 应用程序包含 DynamicLogger.h 头文件
		1. 创建 CDynamicLogger 对象（通常为全局对象）
		2. 调用 CDynamicLogger->Init(...) 初始化日志组件
		3. 使用 CDynamicLogger->Log()/Debug()/Trace()/Info()/Warn()/Error()/Fatal() 等方法写日志
		4. 调用 CDynamicLogger->UnInit(...) 清理日志组件（CDynamicLogger 对象析构时也会自动清理日志组件）

		方法三：（直接用导出函数加载 Logger DLL）
		--------------------------------------------------------------------------------------
		0. 应用程序包含 Logger.h 头文件
		1. 手工调用 ILoger_Create() 和 ILoger_Create() 导出函数创建和销毁 ILogger 对象 
		（注：如果是动态加载，需手工调用 ::LoadLibrary()/::FreeLibrary() 系列 API 函数加载和卸载 Logger DLL）
		
		[
			***** 对于希望通过窗口接收日志信息的 GUI 程序 *****

			A. 日志组件初始化成功后调用 SetGUIWindow(HWND) 设置收日志的窗口
			B. 窗口须响应处理 LOG_MESSAGE 消息
			C. 处理完 LOG_MESSAGE 消息后，调用 ILogger::FreeLogMsg() 销毁接收到的 TLogMsg 
		]

Environment:
		1. Windows 2000 or later (_WIN32_WINNT >= 0x0500)
		2. VC++ 2010 or later

Release:
		1. Logger_C.dll		- Console/MBCS/Release
		2. Logger_CD.dll	- Console/MBCS/Debug
		3. Logger_CU.dll	- Console/Unicode/Release
		4. Logger_CUD.dll	- Console/Unicode/Debug
		5. Logger.dll		- GUI/MBCS/Release
		6. Logger_D.dll		- GUI/MBCS/Debug
		7. Logger_U.dll		- GUI/Unicode/Release
		8. Logger_UD.dll	- GUI/Unicode/Debug

Examples:
		1. TestGUILogger		- GUI 版测试程序		（静态加载）
		2. TestDynamicLogger	- GUI 版测试程序		（动态加载）
		3. TestConsoleLogger	- Console 版测试程序	（静态加载）

******************************************************************************/
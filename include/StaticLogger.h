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

#pragma once

#include "Logger.h"

/************************************************/
/********** ILogger 包装器（智能指针） ***********/
/*********** 用于静态加载 Logger DLL ************/

class LOGGER_API CStaticLogger
{
public:
	// 构造函数：如果 bCreate 为 TRUE，则在构建 CStaticLogger 实例的同时创建 ILogger 对象
	CStaticLogger(BOOL bCreate = TRUE);
	// 析构函数
	~CStaticLogger();
private:
	CStaticLogger(const CStaticLogger&);
	CStaticLogger& operator = (const CStaticLogger&);

public:
	inline void Reset			(ILogger* pLogger);	// 重设其封装的 ILogger 指针
	inline BOOL IsValid			()	const;			// 判断其封装的 ILogger 指针是否非空
	inline ILogger* Get			()	const;			// 获取 ILogger 指针
	inline ILogger& operator *	()	const;			// 获取 ILogger 引用
	inline ILogger* operator ->	()	const;			// 获取 ILogger 指针
	inline operator ILogger*	()	const;			// 转换为 ILogger 指针

private:
	ILogger* m_pLogger;
};

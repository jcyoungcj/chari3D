/* FileVersion.h --  version 0.1.0, 2005-2-19

Copyright (C) 2010

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

使用方法:
假设要取得D:\IPCSERVICE.DLL的版本信息：
CString szRet;
CFileVersion fv("D:\\IPCSERVICE.DLL");
szRet = fv.GetFullVersion();
//...
szRet = fv.GetFileVersion();
//...

*/

#ifndef _FILE_VERSION_H_
#define _FILE_VERSION_H_

#pragma comment(lib, "Version.lib")

class CFileVersion
{
public:
	CFileVersion(CString szPath);
	~CFileVersion();

	//取得所有的文件版本信息，各信息间以\n分隔
	CString GetFullVersion();

	//取得备注
	CString GetComments();
	//取得内部名称
	CString GetInternalName();
	//取得产品名称
	CString GetProductName();
	//取得公司名称
	CString GetCompanyName();
	//取得版权
	CString GetLegalCopyright();
	//取得产品版本
	CString GetProductVersion();
	//取得文件描述
	CString GetFileDescription();
	//取得合法商标
	CString GetLegalTrademarks();
	//取得个人用内部版本说明
	CString GetPrivateBuild();
	//取得文件版本
	CString GetFileVersion();
	//取得源文件名
	CString GetOriginalFilename();
	//取得特殊内部版本说明
	CString GetSpecialBuild();

private:
	CString GetVer(CString szSubBlock);

	CString m_szPath;
};


#endif


#include "stdafx.h"
#include "FileVersion.h"

CFileVersion::CFileVersion(CString szPath)
{
	m_szPath = szPath;
}

CFileVersion::~CFileVersion()
{
}

//取得所有的文件版本信息，各信息间以\n分隔
CString CFileVersion::GetFullVersion()
{
	CString szRet;
	szRet = "文件版本: " + GetFileVersion() + "\n";
	szRet += "说明: " + GetFileDescription() + "\n";
	szRet += "版权: " + GetLegalCopyright() + "\n";
	szRet += "备注: " + GetComments() + "\n";
	szRet += "产品版本: " + GetProductVersion() + "\n";
	szRet += "产品名称: " + GetProductName() + "\n";
	szRet += "个人用内部版本说明: " + GetPrivateBuild() + "\n";
	szRet += "公司名称: " + GetCompanyName() + "\n";
	szRet += "合法商标: " + GetLegalTrademarks() + "\n";
	szRet += "内部名称: " + GetInternalName() + "\n";
	szRet += "特殊内部版本说明: " + GetSpecialBuild() + "\n";
	szRet += "源文件名: " + GetOriginalFilename() + "\n";
	return szRet;
}

//取得备注
CString CFileVersion::GetComments()
{
	return GetVer("Comments");
}

//取得内部名称
CString CFileVersion::GetInternalName()
{
	return GetVer("InternalName");
}

//取得产品名称
CString CFileVersion::GetProductName()
{
	return GetVer("ProductName");
}

//取得公司名称
CString CFileVersion::GetCompanyName()
{
	return GetVer("CompanyName");
}

//取得版权
CString CFileVersion::GetLegalCopyright()
{
	return GetVer("LegalCopyright");
}

//取得产品版本
CString CFileVersion::GetProductVersion()
{
	return GetVer("ProductVersion");
}

//取得文件描述
CString CFileVersion::GetFileDescription()
{
	return GetVer("FileDescription");
}

//取得合法商标
CString CFileVersion::GetLegalTrademarks()
{
	return GetVer("LegalTrademarks");
}

//取得个人内部版本说明
CString CFileVersion::GetPrivateBuild()
{
	return GetVer("PrivateBuild");
}

//取得文件版本
CString CFileVersion::GetFileVersion()
{
	return GetVer("FileVersion");
}

//取得源文件名
CString CFileVersion::GetOriginalFilename()
{
	return GetVer("OriginalFilename");
}

//取得特殊内部版本说明
CString CFileVersion::GetSpecialBuild()
{
	return GetVer("SpecialBuild");
}

CString CFileVersion::GetVer(CString szSubBlock)
{
	CString szRet;

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	DWORD dwHandle;
	DWORD dwInfoSize;

	dwInfoSize = GetFileVersionInfoSize((char*)(LPCTSTR)m_szPath, &dwHandle);

	if (dwInfoSize != 0)
	{
		char* lpData = new char[dwInfoSize];
		if (GetFileVersionInfo((char*)(LPCTSTR)m_szPath, 0, dwInfoSize, lpData))
		{
			UINT cbTranslate = 0;
			VerQueryValue(lpData,
				TEXT("\\VarFileInfo\\Translation"),
				(LPVOID*)&lpTranslate,
				&cbTranslate);

			char SubBlock[256];
			char* lpBuffer;
			UINT dwBytes = 0;
			for (int i = 0; i <(int)(cbTranslate / sizeof(struct LANGANDCODEPAGE)); i++)
			{
				wsprintf(SubBlock,
					TEXT("\\StringFileInfo\\%04x%04x\\%s"),
					lpTranslate[i].wLanguage,
					lpTranslate[i].wCodePage,
					szSubBlock);

				VerQueryValue(lpData,
					SubBlock,
					(LPVOID*)&lpBuffer,
					&dwBytes);

				szRet.Format("%s", lpBuffer);

				break; //only retrieve the first language and code page
			}

		}
		delete[] lpData;
		lpData = NULL;
	}

	return szRet;
}
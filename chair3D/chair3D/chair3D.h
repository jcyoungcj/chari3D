
// chair3D.h : PROJECT_NAME 应用程序的主头文件
//



#if !defined(AFX_GxSingleCamColor_H__42E4C164_A272_45E1_945D_981DE8DEE59C__INCLUDED_)
#define AFX_GxSingleCamColor_H__42E4C164_A272_45E1_945D_981DE8DEE59C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// Cchair3DApp: 
// 有关此类的实现，请参阅 chair3D.cpp
//

class Cchair3DApp : public CWinApp
{
public:
	Cchair3DApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern Cchair3DApp theApp;
#endif
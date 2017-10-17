
// chair3DDlg.cpp : 实现文件
//常见错误
//CString错误：多字符集； 
//如果imread 无法读取图片是由于加载了release的lib
//1641
#include "stdafx.h"
#include "chair3D.h"
#include "chair3DDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE 
static char THIS_FILE[] = __FILE__;
#endif

//result无法读取内存
//---------------------------------------------------------------------------------
                                  /*   初始化参数  */
#define PI 3.14159265;
#define numofLine 18                             //每张标定图片的线数
#define yuzhi 40								//阈值大小
#define size_pixel 0.00375						//像原大小 3.75um；mm为单位
#define angle 60								    //三角法透镜与法线夹角
#define len_base 600							//透镜到基准面的距离
#define F 8									    //透镜到CCD距离，一般按焦距
#define width 1292								//图像宽度
#define height 964								//图像高度
//#define numofpoint 23356                         //离散点的个数=numofLine*图像宽度
//#define N 9                                      //使用标定图片数目；不同高度采集的图片
//#define IS_SNAP_SINGLE 0

double result[height][width];									 //保存每个点的三维高度
//int w[numofLine][];									 //保存每条线的x值，用于确定激光线间距


//----------------------------------------------------------------------------------

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };
	CString	m_strProductVersion;
	CString	m_strLegalCopyRight;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
	m_strProductVersion = _T("");
	m_strLegalCopyRight = _T("");
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cchair3DDlg 对话框



Cchair3DDlg::Cchair3DDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cchair3DDlg::IDD, pParent)
	, m_bDevOpened(FALSE)
	, m_bIsSnap(FALSE)
	, m_bIsSaveImg(FALSE)
	, m_hDevice(NULL)
	, m_pBmpInfo(NULL)
	, m_pBufferRaw(NULL)
	, m_pBufferRGB(NULL)
	, m_nPayLoadSize(0)
	, m_nImageHeight(0)
	, m_nImageWidth(0)
	, m_nPixelColorFilter(GX_COLOR_FILTER_NONE)
	, m_strFilePath("")
	, m_pWnd(NULL)
	, m_hDC(NULL)
{
	PixelFormat = 0;////////////////////////////////////////////////////
	m_yRotate = 0;//////////////////////////////////
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_dBalanceRatio = 0.0;
	m_dGainValue = 0.0;
	m_dShutterValue = 0.0;
	// 初始化参数
	memset(&m_stGainFloatRange, 0, sizeof(GX_FLOAT_RANGE));
	memset(&m_stShutterFloatRange, 0, sizeof(GX_FLOAT_RANGE));
	//memset(&m_stFloatRange,0,sizeof(GX_FLOAT_RANGE));
	memset(m_chBmpBuf, 0, sizeof(m_chBmpBuf));

}

void Cchair3DDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_GAIN, m_dGainValue);
	DDX_Text(pDX, IDC_EDIT_SHUTTER, m_dShutterValue);
}

BEGIN_MESSAGE_MAP(Cchair3DDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()/////////////////没有改函数，opengl无法显示/////////////////////////////////
	ON_BN_CLICKED(IDC_BTN_OPEN_DEVICE, &Cchair3DDlg::OnBnClickedBtnOpenDevice)
	ON_BN_CLICKED(IDC_CALIBRATION, &Cchair3DDlg::OnBnClickedCalibration)
	ON_BN_CLICKED(IDC_BTN_STOP_SNAP, &Cchair3DDlg::OnBnClickedBtnStopSnap)
	ON_BN_CLICKED(IDC_BTN_CLOSE_DEVICE, &Cchair3DDlg::OnBnClickedBtnCloseDevice)
	ON_EN_CHANGE(IDC_EDIT_SHUTTER, &Cchair3DDlg::OnEnChangeEditShutter)
	ON_EN_CHANGE(IDC_EDIT_GAIN, &Cchair3DDlg::OnEnChangeEditGain)
	ON_BN_CLICKED(IDC_SAVE_IMAGE, &Cchair3DDlg::OnBnClickedSaveImage)
	ON_BN_CLICKED(IDC_MESH, &Cchair3DDlg::OnBnClickedMesh)
	ON_BN_CLICKED(IDC_profile, &Cchair3DDlg::OnBnClickedprofile)
	ON_EN_CHANGE(IDC_EDIT3, &Cchair3DDlg::OnEnChangeEdit3)
END_MESSAGE_MAP()


// Cchair3DDlg 消息处理程序

BOOL Cchair3DDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MINIMIZE);

	// TODO:  在此添加额外的初始化代码
	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	//初始化库
	emStatus = GXInitLib();
	if (emStatus != GX_STATUS_SUCCESS)
	{
		ShowErrorString(emStatus);
		exit(0);
	}

	//// 获取当前程序执行路径,设置图像保存的默认路径
	//CString strDate = "";
	//char    szName[MAX_PATH] = { 0 };
	//CString strAppFullName = "";
	//GetModuleFileName(NULL, szName, MAX_PATH);
	//strAppFullName = szName;
	//int nPos = strAppFullName.ReverseFind('\\');
	//m_strFilePath += strAppFullName.Left(nPos);
	//m_strFilePath += "\\GxSingleCamColorImages";

	////获取图像显示窗口的指针和绘图DC
//	m_pWnd = GetDlgItem(IDC_STATIC_SHOW_FRAME);
//	m_hDC = ::GetDC(m_pWnd->m_hWnd);

	//// 更新界面控件
	UpDateUI();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Cchair3DDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		char strFileName[MAX_PATH] = { '\0' };
		GetModuleFileName(NULL, strFileName, MAX_PATH);
		CFileVersion fv(strFileName);
		CAboutDlg dlgAbout;
		dlgAbout.m_strProductVersion = _T("Version: ") + fv.GetProductVersion();//_t 是宏，代码支持unicode编译
		dlgAbout.m_strLegalCopyRight = fv.GetLegalCopyright();
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Cchair3DDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Cchair3DDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//---------------------------------------------------------------------------------
/**
\brief   关闭应用程序函数

\return  无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	KillTimer(0);

	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	// 若未停采则先停止采集
	if (m_bIsSnap)
	{
		// 发送停止采集命令
		emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);

		// 注销回调
		emStatus = GXUnregisterCaptureCallback(m_hDevice);
		m_bIsSnap = FALSE;

		// 释放为采集图像开辟的图像Buffer
		UnPrepareForShowImg();
	}

	//释放pDC
	::ReleaseDC(m_pWnd->m_hWnd, m_hDC);

	// 未关闭设备则关闭设备
	if (m_bDevOpened)
	{
		emStatus = GXCloseDevice(m_hDevice);
		m_bDevOpened = FALSE;
		m_hDevice = NULL;

		// 删除颜色校正相关Buffer资源
		//	if (m_pGammaLut != NULL)
		//	{
		//		delete[] m_pGammaLut;
		//		m_pGammaLut = NULL;
		//	}
		//	if (m_pContrastLut != NULL)
		//	{
		//		delete[] m_pContrastLut;
		//		m_pContrastLut = NULL;
		//	}
	}

	// 关闭设备库
	emStatus = GXCloseLib();

	CDialog::OnClose();
}
//----------------------------------------------------------------------------------
/**
\brief  获取GxIAPI错误描述,并弹出错误提示对话框
\param  emErrorStatus  [in]   错误码

\return 无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::ShowErrorString(GX_STATUS emErrorStatus)
{
	char*     pchErrorInfo = NULL;
	size_t    nSize = 0;
	GX_STATUS emStatus = GX_STATUS_ERROR;

	// 获取错误信息长度，并申请内存空间
	emStatus = GXGetLastError(&emErrorStatus, NULL, &nSize);
	pchErrorInfo = new char[nSize];
	if (NULL == pchErrorInfo)
	{
		return;
	}

	// 获取错误信息，并显示
	emStatus = GXGetLastError(&emErrorStatus, pchErrorInfo, &nSize);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		MessageBox("GXGetLastError接口调用失败！");
	}
	else
	{
		MessageBox((LPCTSTR)pchErrorInfo);
	}

	// 释放申请的内存空间
	if (NULL != pchErrorInfo)
	{
		delete[] pchErrorInfo;
		pchErrorInfo = NULL;
	}
}
//---------------------------------------------------------------------------------
/**
\brief   初始化UI界面

\return  无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::InitUI()
{
	GX_STATUS emStatus = GX_STATUS_ERROR;
	int  nValue = 0;
	// 触发模式下拉选项Combox控件
	//	InitTriggerModeUI();
	//	// 初始化触发源Combox控件
	//	InitTriggerSourceUI();
	// 初始化触发极性Combox控件
	//	InitTriggerActivationUI();
	// 初始化白平衡调节Combox控件
	//	InitEnumUI(GX_ENUM_BALANCE_WHITE_AUTO, (CComboBox *)GetDlgItem(IDC_COMBO_AWB), m_bColorFilter);
	// 初始化白平衡通道Combox控件
	//	InitEnumUI(GX_ENUM_BALANCE_RATIO_SELECTOR, (CComboBox *)GetDlgItem(IDC_COMBO_RATIO_SELECTOR), m_bBalanceRatioSelector);
	// 初始化曝光时间相关控件
	InitShutterUI();
	// 初始化增益相关控件
	InitGainUI();
	// 初始化白平衡系数相关控件
	//	InitWhiteRatioUI();

}
//---------------------------------------------------------------------------------
/**
\brief   初始化组合框UI界面
\param   emFeatureID   [in]    功能码ID
\param   pComboBox     [in]    控件指针
\param   bIsImplement  [in]    设备是否支持emFeatureID代表功能

\return  无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::InitEnumUI(GX_FEATURE_ID_CMD emFeatureID, CComboBox *pComboBox, bool bIsImplement)
{
	// 判断控件
	if ((pComboBox == NULL) || !bIsImplement)
	{
		return;
	}

	GX_ENUM_DESCRIPTION *pEnum = NULL;
	GX_STATUS emStatus = GX_STATUS_ERROR;
	size_t    nbufferSize = 0;
	uint32_t  nEntryNum = 0;
	int64_t   nEnumValue = -1;
	int       nCursel = 0;
	double    dValue = 0;

	// 获取功能的枚举数
	emStatus = GXGetEnumEntryNums(m_hDevice, emFeatureID, &nEntryNum);
	GX_VERIFY(emStatus);

	// 为获取枚举型的功能描述分配空间
	nbufferSize = nEntryNum * sizeof(GX_ENUM_DESCRIPTION);
	pEnum = new GX_ENUM_DESCRIPTION[nEntryNum];
	if (pEnum == NULL)
	{
		MessageBox("分配缓冲区失败！");
		return;
	}

	// 获取功能的枚举描述
	emStatus = GXGetEnumDescription(m_hDevice, emFeatureID, pEnum, &nbufferSize);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		if (pEnum != NULL)
		{
			delete[]pEnum;
			pEnum = NULL;
		}
		ShowErrorString(emStatus);
		return;
	}


	// 获取枚举型的当前值,并设为界面当前显示值
	emStatus = GXGetEnum(m_hDevice, emFeatureID, &nEnumValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		if (pEnum != NULL)
		{
			delete[]pEnum;
			pEnum = NULL;
		}
		ShowErrorString(emStatus);
		return;
	}

	// 初始化当前控件的可选项
	pComboBox->ResetContent();
	for (uint32_t i = 0; i < nEntryNum; i++)
	{
		pComboBox->SetItemData(pComboBox->AddString(pEnum[i].szSymbolic), (uint32_t)pEnum[i].nValue);
		if (pEnum[i].nValue == nEnumValue)
		{
			nCursel = i;
		}
	}

	// 设置当前值为界面的显示值
	pComboBox->SetCurSel(nCursel);

	// 释放函数资源
	if (pEnum != NULL)
	{
		delete[]pEnum;
		pEnum = NULL;
	}
}
//----------------------------------------------------------------------------------
/**
\brief  刷新UI界面

\return 无返回值
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::UpDateUI()
{
	GetDlgItem(IDC_BTN_OPEN_DEVICE)->EnableWindow(!m_bDevOpened);
	GetDlgItem(IDC_BTN_CLOSE_DEVICE)->EnableWindow(m_bDevOpened);
	//	GetDlgItem(IDC_BTN_START_SNAP)->EnableWindow(m_bDevOpened && !m_bIsSnap);
	GetDlgItem(IDC_BTN_STOP_SNAP)->EnableWindow(m_bDevOpened && m_bIsSnap);
	//	GetDlgItem(IDC_BTN_SOFTTRIGGER)->EnableWindow(m_bDevOpened);

	GetDlgItem(IDC_EDIT_GAIN)->EnableWindow(m_bDevOpened);
	GetDlgItem(IDC_EDIT_SHUTTER)->EnableWindow(m_bDevOpened);

}



//----------------------------------------------------------------------------------
/**
\brief   初始化相机:设置默认参数

\return  无
*/
//----------------------------------------------------------------------------------
GX_STATUS Cchair3DDlg::InitDevice()
{
	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	//设置采集模式连续采集
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
	VERIFY_STATUS_RET(emStatus);

	// 已知当前相机支持哪个8位图像数据格式可以直接设置
	// 例如设备支持数据格式GX_PIXEL_FORMAT_BAYER_GR8,则可按照以下代码实现
	// emStatus = GXSetEnum(m_hDevice, GX_ENUM_PIXEL_FORMAT, GX_PIXEL_FORMAT_BAYER_GR8);
	// VERIFY_STATUS_RET(emStatus);

	// 不清楚当前相机的数据格式时，可以调用以下函数将图像数据格式设置为8Bit
	emStatus = SetPixelFormat8bit();

	return emStatus;
}

// ---------------------------------------------------------------------------------
/**
\brief   设置相机的数据格式为8bit

\return  emStatus GX_STATUS_SUCCESS:设置成功，其它:设置失败
*/
// ----------------------------------------------------------------------------------
GX_STATUS Cchair3DDlg::SetPixelFormat8bit()
{
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	int64_t   nPixelSize = 0;
	uint32_t  nEnmuEntry = 0;
	size_t    nBufferSize = 0;
	BOOL      bIs8bit = TRUE;

	GX_ENUM_DESCRIPTION  *pEnumDescription = NULL;
	GX_ENUM_DESCRIPTION  *pEnumTemp = NULL;

	// 获取像素点大小
	emStatus = GXGetEnum(m_hDevice, GX_ENUM_PIXEL_SIZE, &nPixelSize);
	VERIFY_STATUS_RET(emStatus);

	// 判断为8bit时直接返回,否则设置为8bit
	if (nPixelSize == GX_PIXEL_SIZE_BPP8)
	{
		return GX_STATUS_SUCCESS;
	}
	else
	{
		// 获取设备支持的像素格式枚举数
		emStatus = GXGetEnumEntryNums(m_hDevice, GX_ENUM_PIXEL_FORMAT, &nEnmuEntry);
		VERIFY_STATUS_RET(emStatus);

		// 为获取设备支持的像素格式枚举值准备资源
		nBufferSize = nEnmuEntry * sizeof(GX_ENUM_DESCRIPTION);
		pEnumDescription = new GX_ENUM_DESCRIPTION[nEnmuEntry];

		// 获取支持的枚举值
		emStatus = GXGetEnumDescription(m_hDevice, GX_ENUM_PIXEL_FORMAT, pEnumDescription, &nBufferSize);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			if (pEnumDescription != NULL)
			{
				delete[]pEnumDescription;
				pEnumDescription = NULL;
			}

			return emStatus;
		}

		// 遍历设备支持的像素格式,设置像素格式为8bit,
		// 如设备支持的像素格式为Mono10和Mono8则设置其为Mono8
		for (uint32_t i = 0; i<nEnmuEntry; i++)
		{
			if ((pEnumDescription[i].nValue & GX_PIXEL_8BIT) == GX_PIXEL_8BIT)
			{
				emStatus = GXSetEnum(m_hDevice, GX_ENUM_PIXEL_FORMAT, pEnumDescription[i].nValue);
				break;
			}
		}

		// 释放资源
		if (pEnumDescription != NULL)
		{
			delete[]pEnumDescription;
			pEnumDescription = NULL;
		}
	}

	return emStatus;
}
//---------------------------------------------------------------------------------
/**
\brief   回调函数
\param   pFrame         回调参数

\return  无
*/
//----------------------------------------------------------------------------------
void __stdcall Cchair3DDlg::OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM* pFrame)
{
	Cchair3DDlg *pDlg = (Cchair3DDlg*)(pFrame->pUserParam);

	if (pFrame->status == 0)
	{
		memcpy(pDlg->m_pBufferRaw, pFrame->pImgBuf, pFrame->nImgSize);

		// RGB转换
		DxRaw8toRGB24(pDlg->m_pBufferRaw
			, pDlg->m_pBufferRGB
			, (VxUint32)(pDlg->m_nImageWidth)
			, (VxUint32)(pDlg->m_nImageHeight)
			, RAW2RGB_NEIGHBOUR
			, DX_PIXEL_COLOR_FILTER(pDlg->m_nPixelColorFilter)
			, TRUE);

		// 图像质量提升
		//if (pDlg->m_bIsImproveImg)
		//{
		//	//提升图像质量
		//	DxImageImprovment(pDlg->m_pBufferRGB
		//		, pDlg->m_pBufferRGB
		//		, (VxUint32)(pDlg->m_nImageWidth)
		//		, (VxUint32)(pDlg->m_nImageHeight)
		//		, pDlg->m_nColorCorrection
		//		, pDlg->m_pContrastLut
		//		, pDlg->m_pGammaLut);
		//}

		// 显示图像
		pDlg->DrawImg();

		// 图像保存
		if (pDlg->m_bIsSaveImg)
		{
			pDlg->SaveImage();
		}
	}
}
//---------------------------------------------------------------------------------
/**
\brief   将m_pBufferRGB中图像显示到界面

\return  无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::DrawImg()
{
	int nWndWidth = 0;
	int nWndHeight = 0;

	// 为画图做准备
	RECT objRect;
	m_pWnd->GetClientRect(&objRect);
	nWndWidth = objRect.right - objRect.left;
	nWndHeight = objRect.bottom - objRect.top;

	// 必须调用该语句，否则图像出现水纹
	::SetStretchBltMode(m_hDC, COLORONCOLOR);
	::StretchDIBits(m_hDC,
		0,
		0,
		nWndWidth,
		nWndHeight,
		0,
		0,
		(int)m_nImageWidth,
		(int)m_nImageHeight,
		m_pBufferRGB,
		m_pBmpInfo,
		DIB_RGB_COLORS,
		SRCCOPY
		);
}

//---------------------------------------------------------------------------------
/**
\brief   为图像显示准备资源

\return  true:为图像显示准备资源成功  false:准备资源失败
*/
//----------------------------------------------------------------------------------
bool Cchair3DDlg::PrepareForShowImg()
{
	//--------------------------------------------------------------------
	//---------------------------初始化bitmap头---------------------------
	m_pBmpInfo = (BITMAPINFO *)m_chBmpBuf;
	m_pBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBmpInfo->bmiHeader.biWidth = (LONG)m_nImageWidth;
	m_pBmpInfo->bmiHeader.biHeight = (LONG)m_nImageHeight;

	m_pBmpInfo->bmiHeader.biPlanes = 1;
	m_pBmpInfo->bmiHeader.biBitCount = 24;
	m_pBmpInfo->bmiHeader.biCompression = BI_RGB;
	m_pBmpInfo->bmiHeader.biSizeImage = 0;
	m_pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
	m_pBmpInfo->bmiHeader.biClrUsed = 0;
	m_pBmpInfo->bmiHeader.biClrImportant = 0;

	//---------------------------------------------------------------------
	//----------------------------为图像数据分配Buffer---------------------
	//为存储RGB图像数据申请空间
	m_pBufferRGB = new BYTE[(size_t)(m_nImageWidth * m_nImageHeight * 3)];
	if (m_pBufferRGB == NULL)
	{
		return false;
	}

	//为存储原始图像数据申请空间
	m_pBufferRaw = new BYTE[(size_t)m_nPayLoadSize];
	if (m_pBufferRaw == NULL)
	{
		delete[]m_pBufferRGB;
		m_pBufferRGB = NULL;

		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------
/**
\brief   释放为图像显示准备资源

\return  无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::UnPrepareForShowImg()
{
	if (m_pBufferRGB != NULL)
	{
		delete[]m_pBufferRGB;
		m_pBufferRGB = NULL;
	}
	if (m_pBufferRaw != NULL)
	{
		delete[]m_pBufferRaw;
		m_pBufferRaw = NULL;
	}
}
//----------------------------------------------------------------------------------
/**
\brief  点击"打开设备"控件响应函数

\return 无
*/
//----------------------------------------------------------------------------------
uint32_t    nDevNum = 0;
void Cchair3DDlg::OnBnClickedBtnOpenDevice()
{
	// TODO:  在此添加控件通知处理程序代码
	GX_STATUS      emStatus = GX_STATUS_SUCCESS;
	//uint32_t       nDevNum = 0;
	GX_OPEN_PARAM  stOpenParam;
	stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
	stOpenParam.openMode = GX_OPEN_INDEX;
	stOpenParam.pszContent = "1";

	// 枚举设备
	emStatus = GXUpdateDeviceList(&nDevNum, 1000);
	GX_VERIFY(emStatus);

	// 判断设备个数
	if (nDevNum <= 0)
	{
		MessageBox("没有发现设备!");
		return;
	}

	// 如果设备已经打开则关闭,保证相机在初始化出错情况能再次打开
	if (m_hDevice != NULL)
	{
		emStatus = GXCloseDevice(m_hDevice);
		GX_VERIFY(emStatus);
		m_hDevice = NULL;
	}

	// 打开设备
	emStatus = GXOpenDevice(&stOpenParam, &m_hDevice);
	GX_VERIFY(emStatus);
	m_bDevOpened = TRUE;

	// 设置相机的默认参数:采集模式:连续采集,数据格式:8-bit
	emStatus = InitDevice();
	GX_VERIFY(emStatus);

	// 获取设备的宽、高等属性信息
	emStatus = GetDeviceParam();
	GX_VERIFY(emStatus);

	// 获取相机参数,初始化界面控件
	InitUI();

	// 获取图像质量提升参数
	//	GetImageImproveParam();

	//设置定时器更新界面
	SetTimer(0, 1000, NULL);

	// 更新界面控件
	UpDateUI();
	//开始采集
	emStatus = GX_STATUS_ERROR;

	// 初始化BMP头信息、分配Buffer为图像采集做准备
	if (!PrepareForShowImg())
	{
		MessageBox("为图像显示分配资源失败!");
		return;
	}

	//注册回调
	emStatus = GXRegisterCaptureCallback(m_hDevice, this, OnFrameCallbackFun);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		UnPrepareForShowImg();
		ShowErrorString(emStatus);
		return;
	}

	//发开始采集命令
	emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		UnPrepareForShowImg();
		ShowErrorString(emStatus);
		return;
	}

	m_bIsSnap = TRUE;

	// 更新界面UI
	UpDateUI();
}
GX_STATUS Cchair3DDlg::GetDeviceParam()
{
	GX_STATUS emStatus = GX_STATUS_ERROR;
	bool      bColorFliter = false;

	// 获取图像大小
	emStatus = GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &m_nPayLoadSize);
	VERIFY_STATUS_RET(emStatus);

	// 获取宽度
	emStatus = GXGetInt(m_hDevice, GX_INT_WIDTH, &m_nImageWidth);
	VERIFY_STATUS_RET(emStatus);


	// 获取高度
	emStatus = GXGetInt(m_hDevice, GX_INT_HEIGHT, &m_nImageHeight);

	return emStatus;
}
//----------------------------------------------------------------------------------
/**
\brief  点击"标定"控件响应函数

\return 无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::OnBnClickedCalibration()
{

	////先判断是否标定
	//char cal[MAX_PATH];
	//_getcwd(cal, MAX_PATH);
	//CString cs = cal;
	//cs += "/标定";
	//
	//ifstream fin(cs + "/y_index.yml");
	//if (fin){
	//	MessageBox("已经标定！无需重复标定！");
	//}
	//else{
	//	/********************Calibration******************************/
	//	//Rect rect;
	//	//rect = Rect(0, 0, 1292, 964);//-------图像大小--------------修改------------------
	//	string index;
	//	string s2;
	//	for (int line = 1; line <= numofLine; line++){    //line用来标识是第几行光线
	//		stringstream ss;
	//		ss << line;
	//		ss >> index;
	//		ss.str("");
	//		//		cout<<index<<endl;
	//		string table = cs;
	//		table = table + "/table" + index + ".yml";
	//		float high[numofLine + 2] = { 0 };  //保存了不同标定平面的实际高度--------------改变------------------
	//		Mat	imgMap;  //
	//		ifstream fin(cs + "/high.in");
	//		string filename;
	//		//for (int i = 0; i < N;i++){
	//		//	fin >> high[i];
	//		//	if (high[i]<1e-5)     //高度值中不应存在负值
	//		//		high[i] = 250;      //用250暂时替代0便于区分其他区域
	//		//}
	//		//fin.close();

	//		for (int i = 0; i < N; i++)
	//		{

	//			fin >> high[i];
	//			if (high[i]<1e-5)     //高度值中不应存在负值
	//				high[i] = 250;      //用250暂时替代0便于区分其他区域

	//			string type = ".bmp";
	//			stringstream ss2;
	//			ss2 << i;
	//			ss2 >> s2;
	//			ss.str("");
	//			filename = cs;//从0开始的10张标定图
	//			filename = filename + "/" + s2;
	//			filename += type;
	//			Mat img = imread(filename, 0);//单通道
	//			//imshow("img",img);
	//			//waitKey(0);
	//			if (img.empty())
	//			{
	//				MessageBox("标定图片无法检索到！");
	//			}

	//			Mat img_sk = skeleton(img, high[i], line);   //每张图片处理：骨化激光线，传入图片高度
	//			if (i == 0)
	//				imgMap = img_sk;
	//			else
	//				imgMap += img_sk;

	//		}
	//		indexTable(imgMap, N, table);//生成table文件
	//	}
	//	//生成y_index文件
	//	bool bl = false;//false未获取y索引表;如果已经生成， 不需要跑前面的代码
	//	string file1 = cs + "/0.bmp"; //0.bmp假定为基准平面
	//	Mat imgRoi1 = imread(file1, 0);//0代表灰度图像
	//	Mat imgblur1;
	//	threshold(imgRoi1, imgblur1, yuzhi, 255, CV_THRESH_BINARY);//-------阈值修改---------------
	//	medianBlur(imgblur1, imgblur1, 3);//中值滤波
	//	medianBlur(imgblur1, imgblur1, 3);//图像平滑处理
	//	dilate(imgblur1, imgblur1, (5, 5));//膨胀处理
	//	int w1 = imgRoi1.cols, l1 = imgRoi1.rows;//col 竖列
	//
	//	Mat y_index1 = Mat::zeros(numofLine + 2, m_nImageWidth, CV_32FC1);
	//	if (!bl){
	//		for (int j = 0; j < w1; j++)//1292
	//		{
	//			int isLaser = 0;
	//			int up1 = 0;
	//			int buttom1 = 0;
	//			int line_panduan1 = 0; //用于匹配line
	//			for (int i = 0; i < l1; i++)//964
	//			{
	//				if (imgblur1.at<uchar>(i, j) >yuzhi && !isLaser) //-------修改------
	//				{
	//					isLaser = 1;
	//					++line_panduan1;
	//					up1 = i;
	//				}
	//				else if (imgblur1.at<uchar>(i, j) <yuzhi && isLaser)
	//				{
	//					buttom1 = i - 1;
	//					y_index1.at<float>(line_panduan1 - 1, j) = (up1 + buttom1) / 2.0;
	//					isLaser = 0;
	//				}
	//			}
	//		}
	//		//保存索引的y
	//		string s = cs; s += "/y_index.yml";
	//		FileStorage fs(s, FileStorage::WRITE);
	//		fs << "TABLE" << y_index1;
	//		fs.release();
	//	}
	//	MessageBox("标定完成！");
	//}
}

//----------------------------------------------------------------------------------
/**
\brief  点击"停止采集"控件响应函数

\return 无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::OnBnClickedBtnStopSnap()
{
	// TODO:  在此添加控件通知处理程序代码
	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	//发送停止采集命令
	emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
	GX_VERIFY(emStatus);

	//注销回调
	emStatus = GXUnregisterCaptureCallback(m_hDevice);
	GX_VERIFY(emStatus);

	m_bIsSnap = FALSE;

	// 释放为采集图像开辟的图像Buffer；如果释放，在暂停后无法保存图像
	//	UnPrepareForShowImg();

	// 更新界面UI
	UpDateUI();
}

//----------------------------------------------------------------------------------
/**
\brief  点击"关闭设备"控件响应函数

\return 无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::OnBnClickedBtnCloseDevice()
{
	// TODO:  在此添加控件通知处理程序代码
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	// 未停采时则停止采集
	if (m_bIsSnap)
	{
		//发送停止采集命令
		emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			// 错误处理
		}
		//注销回调
		emStatus = GXUnregisterCaptureCallback(m_hDevice);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			// 错误处理
		}
		m_bIsSnap = FALSE;
		// 释放为采集图像开辟的图像Buffer
		UnPrepareForShowImg();
	}
	emStatus = GXCloseDevice(m_hDevice);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		// 错误处理
	}
	m_bDevOpened = FALSE;
	m_hDevice = NULL;
	// 删除颜色校正相关Buffer资源
	//if (m_pGammaLut != NULL)
	//{
	//	delete[] m_pGammaLut;
	//	m_pGammaLut = NULL;
	//}
	//if (m_pContrastLut != NULL)
	//{
	//	delete[] m_pContrastLut;
	//	m_pContrastLut = NULL;
	//}
	KillTimer(0);
	// 更新界面UI
	UpDateUI();
}
//---------------------------------------------------------------------------------
/**
\brief   初始化曝光相关控件:Static:显示范围 Edit:输入值

\return  无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::InitShutterUI()
{
	CStatic     *pStcShutterShow = (CStatic *)GetDlgItem(IDC_STC_SHUTTER_SHOW);
	CEdit       *pEditShutterVal = (CEdit *)GetDlgItem(IDC_EDIT_SHUTTER);
	int          nPos = 0;

	// 判断控件的有效性
	if ((pEditShutterVal == NULL) || (pStcShutterShow == NULL))
	{
		return;
	}

	GX_STATUS      emStatus = GX_STATUS_ERROR;
	CString         strTemp = "";
	double    dShutterValue = 0.0;

	// 获取浮点型范围,并初始化SliderCtrl控件和static范围提示框
	emStatus = GXGetFloatRange(m_hDevice, GX_FLOAT_EXPOSURE_TIME, &m_stShutterFloatRange);
	GX_VERIFY(emStatus);

	strTemp.Format("曝光(%.4f~%.4f)%s", m_stShutterFloatRange.dMin, m_stShutterFloatRange.dMax, m_stShutterFloatRange.szUnit);
	pStcShutterShow->SetWindowText(strTemp);

	// 获取当前值并将当前值更新到界面
	emStatus = GXGetFloat(m_hDevice, GX_FLOAT_EXPOSURE_TIME, &dShutterValue);
	GX_VERIFY(emStatus);

	m_dShutterValue = dShutterValue;

	UpdateData(FALSE);
}
// ---------------------------------------------------------------------- -
/**
\brief   分发函数主要处理曝光和增益的Edit框响应回车键的消息
\param   pMsg  消息结构体

\return 成功:TRUE   失败:FALSE
*/
//-----------------------------------------------------------------------
BOOL Cchair3DDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	CWnd  *pWnd = NULL;
	int   nCtrlID = 0;             //< 保存获取的控件ID

	//判断是否是键盘回车消息
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN))
	{
		//获取当前拥有输入焦点的窗口(控件)指针
		pWnd = GetFocus();

		//获得当前焦点的控件ID
		nCtrlID = pWnd->GetDlgCtrlID();

		//判断ID类型
		switch (nCtrlID)
		{
			//	case IDC_EDIT_BALANCE_RATIO:
		case IDC_EDIT_SHUTTER:
		case IDC_EDIT_GAIN:

			//失去焦点
			SetFocus();

			break;

		default:
			break;
		}

		return TRUE;
	}
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE))
	{
		return  TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

//-----------------------------------------------------------------------
/**
\brief    控制曝光时间的Edit框失去焦点的响应函数

\return   无
*/
//-----------------------------------------------------------------------
void Cchair3DDlg::OnEnChangeEditShutter()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	//判断句柄是否有效，避免设备掉线后关闭程序出现的异常
	if (m_hDevice == NULL)
	{
		return;
	}

	UpdateData();
	GX_STATUS status = GX_STATUS_SUCCESS;

	//判断输入值是否在曝光时间的范围内
	//若大于最大值则将曝光值设为最大值
	if (m_dShutterValue > m_stShutterFloatRange.dMax)
	{
		m_dShutterValue = m_stShutterFloatRange.dMax;
	}
	//若小于最小值将曝光值设为最小值
	if (m_dShutterValue < m_stShutterFloatRange.dMin)
	{
		m_dShutterValue = m_stShutterFloatRange.dMin;
	}
	status = GXSetFloat(m_hDevice, GX_FLOAT_EXPOSURE_TIME, m_dShutterValue);
	GX_VERIFY(status);

	UpdateData(FALSE);

}


//---------------------------------------------------------------------------------
/**
\brief   初始化增益相关控件: Static:显示范围 Edit:输入值

\return  无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::InitGainUI()
{
	CStatic     *pStcGain = (CStatic *)GetDlgItem(IDC_STC_GAIN);
	CEdit       *pEditGainVal = (CEdit *)GetDlgItem(IDC_EDIT_GAIN);
	int          nPos = 0;

	if ((pStcGain == NULL) || (pEditGainVal == NULL))
	{
		return;
	}

	GX_STATUS emStatus = GX_STATUS_ERROR;
	CString   strRange = "";
	double    dGainVal = 0;

	// 获取浮点型范围,并初始化SliderCtrl控件和static范围提示框
	emStatus = GXGetFloatRange(m_hDevice, GX_FLOAT_GAIN, &m_stGainFloatRange);
	GX_VERIFY(emStatus);

	// 显示范围
	strRange.Format("增益(%.4f~%.4f)%s", m_stGainFloatRange.dMin, m_stGainFloatRange.dMax, m_stGainFloatRange.szUnit);
	pStcGain->SetWindowText(strRange);

	// 获取当前值并更新到界面
	emStatus = GXGetFloat(m_hDevice, GX_FLOAT_GAIN, &dGainVal);
	GX_VERIFY(emStatus);

	m_dGainValue = dGainVal;

	UpdateData(FALSE);
}

//--------------------------------------------------------------------
/**
\brief   控制增益值的Edit框失去焦点的响应函数

\return  无
*/
//--------------------------------------------------------------------
void Cchair3DDlg::OnEnChangeEditGain()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	//判断句柄是否有效，避免设备掉线后关闭程序出现的异常
	if (m_hDevice == NULL)
	{
		return;
	}

	UpdateData();
	GX_STATUS status = GX_STATUS_SUCCESS;

	//判断输入值是否在增益值的范围内
	//若输入的值大于最大值则将增益值设置成最大值
	if (m_dGainValue > m_stGainFloatRange.dMax)
	{
		m_dGainValue = m_stGainFloatRange.dMax;
	}

	//若输入的值小于最小值则将增益的值设置成最小值
	if (m_dGainValue < m_stGainFloatRange.dMin)
	{
		m_dGainValue = m_stGainFloatRange.dMin;
	}

	status = GXSetFloat(m_hDevice, GX_FLOAT_GAIN, m_dGainValue);
	GX_VERIFY(status);

	UpdateData(FALSE);
}


//----------------------------------------------------------------------------------
/**
\brief   将m_pBufferRGB图像数据保存成BMP图片

\return  无
*/
//----------------------------------------------------------------------------------
void Cchair3DDlg::SaveImage()//保存图像调用的函数
{
	// TODO: Add your control notification handler code here
	DWORD		         dwImageSize = (DWORD)(m_nImageWidth * m_nImageHeight * 3);
	BITMAPFILEHEADER     stBfh = { 0 };
	DWORD		         dwBytesRead = 0;
	SYSTEMTIME           sysTime;
	CString              strFileName = "";

	char cal[MAX_PATH];
	_getcwd(cal, MAX_PATH);
	CString cs = cal;
	cs += "/测量";
	m_strFilePath = cs;
	//创建保存图像的文件夹
	BOOL bRet = CreateDirectory(m_strFilePath, NULL);
	if (bRet)
	{
		SetFileAttributes(m_strFilePath, FILE_ATTRIBUTE_NORMAL);
	}

	// 获取当前时间为图像保存的默认名称
	GetLocalTime(&sysTime);
	strFileName.Format("%s\\%d.bmp", m_strFilePath, 1);

	stBfh.bfType = (WORD)'M' << 8 | 'B';			 //定义文件类型
	stBfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);	//定义文件头大小彩色
	stBfh.bfSize = stBfh.bfOffBits + dwImageSize; //文件大小

	//创建文件
	HANDLE hFile = ::CreateFile(strFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::WriteFile(hFile, &stBfh, sizeof(BITMAPFILEHEADER), &dwBytesRead, NULL);
		::WriteFile(hFile, m_pBmpInfo, sizeof(BITMAPINFOHEADER), &dwBytesRead, NULL);  //彩色
		::WriteFile(hFile, m_pBufferRGB, dwImageSize, &dwBytesRead, NULL);
		CloseHandle(hFile);
	}
}
void Cchair3DDlg::OnBnClickedSaveImage()
{
	// TODO:  
	if (nDevNum <= 0)
	{
		MessageBox("设备未打开!");
		return;
	}
	else{
		SaveImage();
		MessageBox("保存成功!");
	}
}

//----------------------------------------------------------------------------------
/**
\       将三维重构；需要和matlab结合

\return  无
*/
//----------------------------------------------------------------------------------
//double result[numofpoint][3];
//ofstream of;
//int point = 0;
////-----------------------------近邻搜索-----------------

void dfs(Mat guhua, int x, int y, int w, int l, int line_panduan,Mat image_pixel){
	if (x >= 0 && x<l && y - 1 >= 0 && y <w && guhua.at<double>(x, y - 1)>0){
		image_pixel.at<double>(line_panduan, y-1) = guhua.at<double>(x,y - 1);
		return dfs(guhua, x, y - 1, w, l, line_panduan, image_pixel);
	}
	else if (x - 2 >= 0 && x<l && y - 1 >= 0 && y <w && guhua.at<double>(x - 2, y - 1)>0){
		image_pixel.at<double>(line_panduan, y - 1) = guhua.at<double>(x - 2, y - 1);

		//int x1 = y_index.at<float>(line_panduan - 1, y - 1);
		//if (table.at<float>(x - 2, y - 1)>0){
		//	result[point][0] = x1; result[point][1] = y - 1; result[point][2] = table.at<float>(x - 2, y - 1);
		//	point++;
		//	of << x1 << " " << y - 1 << " " << table.at<float>(x - 2, y - 1) << endl;
		//}
		return dfs(guhua, x - 2, y - 1, w, l, line_panduan, image_pixel);
	}
	else if (x - 1 >= 0 && x<l && y - 1 >= 0 && y <w && guhua.at<float>(x - 1, y - 1)>0){
		image_pixel.at<double>(line_panduan, y - 1) = guhua.at<double>(x - 1, y - 1);
		return dfs(guhua, x - 1, y - 1, w, l, line_panduan, image_pixel);
	}
	else if (x >= 0 && x + 2<l && y - 1 >= 0 && y <w && guhua.at<float>(x + 2, y - 1)>0){
		image_pixel.at<double>(line_panduan, y - 1) = guhua.at<double>(x + 2, y - 1);
		return dfs(guhua, x +2, y - 1, w, l, line_panduan, image_pixel);
	}
	else if (x >= 0 && x + 1<l && y - 1 >= 0 && y<w && guhua.at<float>(x + 1, y - 1)>0){
		image_pixel.at<double>(line_panduan, y - 1) = guhua.at<double>(x +1, y - 1);
		return dfs(guhua, x + 1, y - 1, w, l, line_panduan, image_pixel);
	}
}
//

void dfs2(Mat guhua, int x, int y, int w, int l, int line_panduan, Mat image_pixel){
	if (x >= 0 && x<l && y >= 0 && y+1 <w && guhua.at<double>(x, y + 1)>0){
		image_pixel.at<double>(line_panduan, y + 1) = guhua.at<double>(x, y + 1);
		return dfs2(guhua, x, y + 1, w, l, line_panduan, image_pixel);
	}
	else if (x - 2 >= 0 && x<l && y >= 0 && y + 1<w && guhua.at<double>(x - 2, y + 1)>yuzhi){
		image_pixel.at<double>(line_panduan, y + 1) = guhua.at<double>(x - 2, y + 1);
		return dfs2(guhua, x - 2, y + 1, w, l, line_panduan, image_pixel);
	}
	else if (x - 1 >= 0 && x<l && y >= 0 && y + 1<w && guhua.at<double>(x - 1, y + 1)>yuzhi){
		image_pixel.at<double>(line_panduan, y + 1) = guhua.at<double>(x - 1, y + 1);
		return dfs2(guhua, x - 1, y + 1, w, l, line_panduan, image_pixel);
	}
	else if (x >= 0 && x + 2<l && y >= 0 && y + 1<w && guhua.at<double>(x + 2, y + 1)>yuzhi){
		image_pixel.at<double>(line_panduan, y + 1) = guhua.at<double>(x + 2, y + 1);
		return dfs2(guhua, x + 2, y + 1, w, l, line_panduan, image_pixel);
	}
	else if (x >= 0 && x + 1 < l && y >= 0 && y + 1 < w && guhua.at<double>(x + 1, y + 1) > yuzhi){
		image_pixel.at<double>(line_panduan, y + 1) = guhua.at<double>(x + 1, y + 1);
		return dfs2(guhua, x + 1, y + 1, w, l, line_panduan, image_pixel);
	}
}
//---------------------------------------------------





//-------------------------------------------------------
BOOL Cchair3DDlg::SetWindowPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pixelDesc;
	pixelDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelDesc.nVersion = 1;
	pixelDesc.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER |
		PFD_TYPE_RGBA;
	pixelDesc.iPixelType = PFD_TYPE_RGBA;
	pixelDesc.cColorBits = 32;
	pixelDesc.cRedBits = 0;
	pixelDesc.cRedShift = 0;
	pixelDesc.cGreenBits = 0;
	pixelDesc.cGreenShift = 0;
	pixelDesc.cBlueBits = 0;
	pixelDesc.cBlueShift = 0;
	pixelDesc.cAlphaBits = 0;
	pixelDesc.cAlphaShift = 0;
	pixelDesc.cAccumBits = 0;
	pixelDesc.cAccumRedBits = 0;
	pixelDesc.cAccumGreenBits = 0;
	pixelDesc.cAccumBlueBits = 0;
	pixelDesc.cAccumAlphaBits = 0;
	pixelDesc.cDepthBits = 0;
	pixelDesc.cStencilBits = 1;
	pixelDesc.cAuxBuffers = 0;
	pixelDesc.iLayerType = PFD_MAIN_PLANE;
	pixelDesc.bReserved = 0;
	pixelDesc.dwLayerMask = 0;
	pixelDesc.dwVisibleMask = 0;
	pixelDesc.dwDamageMask = 0;
	PixelFormat = ChoosePixelFormat(hDC, &pixelDesc);
	if (PixelFormat == 0) // Choose default 
	{
		PixelFormat = 1;
		if (DescribePixelFormat(hDC, PixelFormat,
			sizeof(PIXELFORMATDESCRIPTOR), &pixelDesc) == 0)
		{
			return FALSE;
		}
	}
	if (SetPixelFormat(hDC, PixelFormat, &pixelDesc) == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL Cchair3DDlg::CreateViewGLContext(HDC hDC)
{
	hrenderRC = wglCreateContext(hDC);
	if (hrenderRC == NULL)
		return FALSE;
	if (wglMakeCurrent(hDC, hrenderRC) == FALSE)
		return FALSE;
	return TRUE;
}

void Cchair3DDlg::RenderScene()
{

	///////////////////////////////////////////////// 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -6.0f);      // Move Left 1.5 Units And Into The Screen 6.0 
	glRotated(m_yRotate, 0.0, 1.0, 0.0);
	glBegin(GL_TRIANGLES); // Drawing Using Triangles 

	//float x, y, z;
	//glPointSize(1.0);
	//glBegin(GL_POINTS);//GL_POINTS  
	//ifstream file("out.txt");
	//while (file){
	//	file >> x >> y >> z;
	//	x = (x - 512) / 200;
	//	y = (y - 484) / 200;//2
	//	z = (255 - z) / 300;
	//	glColor3f(1, 0, 0);
	//	glVertex3f(x, y, z);//参数的绝对值只能小于1，大于1就没法显示
	//}

	//int len = sizeof(result) / sizeof(int);
	//int h1 = sizeof(result[0]) / sizeof(int);
	//int w1 = len / h1;
	for (int i = 0; i < numofLine;i++){
		for (int j = 0; j < width;j++){
			if (result[i][j]>0){
				double z = result[i][j];
				i = (i - 512) / 200;
				j = (j - 484) / 200;//2
				z = (255 - z) / 300;
				glColor3f(1, 0, 0);
				glVertex3f(i, j, z);//参数的绝对值只能小于1，大于1就没法显示
			}
		}
	}
//	glVertex3f(1, 1, 1); glVertex3f(1,0, 0); glVertex3f(0, 0, 1); glVertex3f(1, 0, 1);
	glEnd();
	SwapBuffers(m_hDC);//------
}
void Cchair3DDlg::OnTimer(UINT_PTR nIDEvent) //实时绘制场景 //  UINT nIDEvent
{
	// TODO: Add your message handler code here and/or call default 
	RenderScene(); 
	
	m_yRotate += 3;
	CDialog::OnTimer(nIDEvent);
}

BOOL Cchair3DDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	wglDeleteContext(hrenderRC);
	return CDialog::DestroyWindow();
}
//激光线骨架化
 Mat Cchair3DDlg::deal(Mat mat){
	threshold(mat, mat, yuzhi, 255, CV_THRESH_BINARY); 
	medianBlur(mat, mat, 3);//中值滤波
	dilate(mat, mat, (5, 5));//膨胀

	Mat out(height,width, CV_32FC1, Scalar(0));//行数，列数

	int w = width, l = height;
	int isLaser, up, buttom;
	for (int j = 0; j < w; j++)//1292
	{
		isLaser = 0;
		up = 0;
		buttom = 0;
		for (int i = 0; i <l; i++)//l:964
		{
			if (mat.at<uchar>(i, j) > yuzhi && !isLaser)
			{
				isLaser = 1;
				up = i;
			}
			else if (mat.at<uchar>(i, j) <yuzhi && isLaser)
			{
				buttom = i - 1;
				isLaser = 0;                                  
				out.at<double>((up + buttom) / 2, j) = (up + buttom) / 2.0;
			}
		}
	}
	return out;
}

void Cchair3DDlg::OnBnClickedMesh()
{
	// TODO:  在此添加控件通知处理程序代码

	char cal[MAX_PATH];
	_getcwd(cal, MAX_PATH);
	string file1 = cal; //实验采集图只有一张
	string base0 = file1 + "\\测量\\0.bmp";
	string read1 = file1 + "\\测量\\1.bmp";

	Mat base_0 = imread(base0,0);//0代表灰度图像
	Mat imgRoi = imread(read1, 0);//0代表灰度图像

	if (base_0.empty()){
		MessageBox("请先采集基准平面图像并保存为0.bmp");
		return;
	}
	if (imgRoi.empty()){
		MessageBox("采集图像为空");
		return;
	}

	int w = width, l = height;
//	////	int w = 1292, l = 964;//col 竖列w=1292

	ofstream of;
	of.open("out.txt");
	Mat base = deal(base_0);								//激光线骨架化
	Mat image = deal(imgRoi);								
	Mat base_pixel(numofLine, width, CV_32FC1, Scalar(0));
	Mat image_pixel(numofLine, width, CV_32FC1, Scalar(0));					//采用最近邻搜索来避免匹配问题
	for (int j = 0; j < w;j++){
		int line = 0;
		for (int i = 0; i < l;i++){
			if (base.at<double>(i,j)>0){
				base_pixel.at<double>(line,j) = base.at<double>(i, j);
				line++;
			}
		}

	}
	//寻找start
	int start;
	for (int j = 0; j < w; j++)//1292
	{
		int line_panduan = 0;//用于匹配line
		for (int i = 0; i <l; i++)//l:964
		{
			if (image.at<double>(i,j) > 0){												///erroe
				++line_panduan;
			}
		}
		if (line_panduan == numofLine){ start = j; break; }
	}

	////dfs
	int line_panduan = 0;
	for (int i = 0; i <l; i++){
		if (image.at<float>(i, start) > 0){	
			if (line_panduan <= numofLine){
				int k = i; int k2 = i;													
				dfs(image, k, start, w, l, line_panduan, image_pixel);						//left
				dfs2(image, k2, start, w, l, line_panduan, image_pixel);					//right
			}
			++line_panduan;
		}
	}

	//三维数据
	for (int i = 0; i < numofLine;i++){
		for (int j = 0; j < w && image_pixel.at<double>(i,j)>0;j++){					 //线条存在断裂的情况
			int pixel = abs(image_pixel.at<double>(i,j) - base_pixel.at<double>(i,j));                     //像素差
			double h = len_base * pixel * size_pixel* 1 / ( F * 0.6 - pixel * size_pixel * cos(angle));      //sin(angle)=0.6直射式激光三角法计算像素偏移量对应的深度值                                            //直射式激光三角法  修改
			int x = base.at<double>(i,j);
			result[x][j] = h;										    //x的值为从数组获取-----修
			of << x << " " << j << " " << h << endl;					//
		}
	}

	//////////////////////三维显示  OpenGL  //////////////////////////////////////

	CWnd *m_pWnd = GetDlgItem(IDC_STATIC_SHOW_FRAME);
	m_hDC = ::GetDC(m_pWnd->m_hWnd);
	if (SetWindowPixelFormat(m_hDC) == FALSE)
		return;
	if (CreateViewGLContext(m_hDC) == FALSE)
		return;

	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
	/////////////////////////////////////////// 
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, 300, 330);//窗口位置和大小  259, 231
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();//重置当前指定的矩阵为单位矩阵.
	gluPerspective(60, 1, 0.1, 100.0);  //观察的视景体在世界坐标系中的具体大小
	glMatrixMode(GL_MODELVIEW);//指定哪一个矩阵是当前矩阵
	glLoadIdentity();
	glShadeModel(GL_SMOOTH);       // Enable Smooth Shading 
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);    // Black Background 
	glClearDepth(1.0f);         // Depth Buffer Setup --------
	glEnable(GL_DEPTH_TEST);       // Enables Depth Testing 
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do 
	///////////////////////////////////////////////////////////////////////// 
	glEnableClientState(GL_VERTEX_ARRAY);//启用颜色矩阵
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	SetTimer(1, 10, 0);

}

//----------------------------------------------
//							轮廓度计算
//----------------------------------------------
void Cchair3DDlg::OnBnClickedprofile()
{
	// TODO:  在此添加控件通知处理程序代码
}


void Cchair3DDlg::OnEnChangeEdit3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}




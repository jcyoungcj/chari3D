
// chair3DDlg.h : 头文件
//
#if !defined(AFX_GxSingleCamColorDLG_H__D8D963C2_3BE2_44EC_B0EB_FCE016DDD6FA__INCLUDED_)
#define AFX_GxSingleCamColorDLG_H__D8D963C2_3BE2_44EC_B0EB_FCE016DDD6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define  WHITE_BALANCE_RNTRY        3         ///< 自动白平衡枚举数
//#define  DEFAULT_CONTRAST           0         ///< 默认对比度值
//#define  DEFAULT_GAMMA              1         ///< 默认Gamma值
//#define  DEFAULT_COLOR_CORRECTION   0         ///< 默认颜色校正值

///< 错误提示函数宏定义
#define  GX_VERIFY(emStatus) \
if (emStatus != GX_STATUS_SUCCESS)\
							{\
							ShowErrorString(emStatus); \
							return; \
							}
///< 判断返回值宏定义
#define VERIFY_STATUS_RET(emStatus) \
if (emStatus != GX_STATUS_SUCCESS) \
								   {\
								   return emStatus; \
								   }
#include "fit.h"
#include "engine.h"
#pragma comment( lib, "libeng.lib" )
#pragma comment( lib, "libmx.lib" )
#pragma comment( lib, "libmat.lib" )
#pragma comment( lib, "mclmcrrt.lib" )
#pragma comment( lib, "fit.lib")
//-------
#include "GxIAPI.h"
#include "DxImageProc.h"
#include "FileVersion.h"
#include <time.h>
#include "skeleton.h"
#include "indexTable.h"
#include <string>
#include <direct.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <cv.h>
using namespace cv;
using namespace std;

#pragma once//防止某个头文件被多次加载

// Cchair3DDlg 对话框
class Cchair3DDlg : public CDialogEx
{
// 构造
public:
	Cchair3DDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CHAIR3D_DIALOG };
public:
	GX_DEV_HANDLE       m_hDevice;              ///< 设备句柄

	BITMAPINFO          *m_pBmpInfo;	        ///< BITMAPINFO 结构指针，显示图像时使用
	BYTE                *m_pBufferRaw;          ///< 原始图像数据
	BYTE                *m_pBufferRGB;	        ///< RGB图像数据，用于显示和保存bmp图像
	char                 m_chBmpBuf[2048];	    ///< BIMTAPINFO 存储缓冲区，m_pBmpInfo即指向此缓冲区

	int64_t             m_nPayLoadSize;         ///< 图像大小
	int64_t             m_nImageHeight;         ///< 原始图像高
	int64_t             m_nImageWidth;          ///< 原始图像宽
	int64_t             m_nPixelColorFilter;    ///< Bayer格式

	GX_FLOAT_RANGE      m_stShutterFloatRange;  ///< 保存曝光时间的范围
	GX_FLOAT_RANGE      m_stGainFloatRange;     ///< 保存增益值的范围

	CString             m_strFilePath;          ///< BMP图像默认保存路径
	BOOL                m_bIsSaveImg;           ///< 标识是否保存图像
	BOOL                m_bDevOpened;           ///< 标识是否已打开设备
	BOOL                m_bIsSnap;              ///< 标识是否开始采集
	//	BOOL                m_bIsImproveImg;        ///< 标识是否进行图像质量提升

	CWnd                *m_pWnd;                ///< 显示图像窗口(控件)指针
	HDC                 m_hDC;                  ///< 绘制图像DC句柄
public:
	Mat deal(Mat mat);
	bool PrepareForShowImg();///  为图像显示准备资源
	void InitUI();/// 获取设备参数,初始化界面控件
	void UpDateUI();/// 刷新UI界面
	static void __stdcall OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM* pFrame);/// 回调函数
	void UnPrepareForShowImg();///  释放为图像显示准备资源	
	void DrawImg();/// 显示图像
	void SaveImage();/// 保存图像
//	void SaveImage2(); //自己定义的保存路径
	void ShowErrorString(GX_STATUS emErrorStatus);/// 错误信息提示
	void InitEnumUI(GX_FEATURE_ID_CMD emFeatureID, CComboBox *pComboBox, bool bIsImplement);/// 初始化枚举类型UI界面
	void InitGainUI();/// 初始化增益等相关控件	
	void InitShutterUI();/// 初始化曝光相关控件
	GX_STATUS InitDevice();/// 初始化相机:设置默认参数
	GX_STATUS SetPixelFormat8bit();/// 设置相机的数据格式为8bit	
	GX_STATUS GetDeviceParam();/// 获取设备的宽、高等属性信息

	double  m_dBalanceRatio;
	double	m_dGainValue;
	double	m_dShutterValue;

	BOOL SetWindowPixelFormat(HDC hDC);   //设定象素格式 
	BOOL CreateViewGLContext(HDC hDC);     //View GL Context                             
	void RenderScene();  //绘制场景 
//	HDC hrenderDC;      //DC 
	HGLRC hrenderRC;  //RC 
	float m_yRotate;       //转速 
	int PixelFormat;
	virtual BOOL DestroyWindow();

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

protected:
	afx_msg void OnPaint();
protected:
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnOpenDevice();
	afx_msg void OnBnClickedBtnCloseDevice();
	afx_msg void OnBnClickedBtnStopSnap();
	afx_msg void OnBnClickedBtnStartSnap();
	afx_msg void OnBnClickedCheckSaveBmp();
	afx_msg void OnKillfocusEditShutter();
	afx_msg void OnKillfocusEditGain();
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBinOpenDevice();
	afx_msg void OnBnClickedCalibration();
	afx_msg void OnBnClickedBinStopSnap();
	afx_msg void OnBnClickedBinCloseDevice();
	afx_msg void OnEnChangeEditShutter();
	afx_msg void OnEnChangeEditGain();
	afx_msg void OnBnClickedSaveImage();
	afx_msg void OnBnClickedMesh();
	afx_msg void OnBnClickedprofile();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
#endif
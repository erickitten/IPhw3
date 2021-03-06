
// IPhw3Dlg.h : 標頭檔
//

#pragma once
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
#include "afxwin.h"

#include "ImageProcesser.h"

// CIPhw3Dlg 對話方塊
class CIPhw3Dlg : public CDialogEx
{
// 建構
public:
	CIPhw3Dlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_IPHW3_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;
	ImageProcesser ip;
	cv::VideoCapture vidIn;
	static const UINT ID_TIMER_SECOND = 0x1002;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()	
public:
	afx_msg	LRESULT	OnKickIdle(WPARAM,LPARAM);
	afx_msg void OnBnClickedLoad();
	CString static_resultstr;
	CStatic resImgControl;
	CStatic binImgControl;
	afx_msg void OnBnClickedGamebtn();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int countDown;
	CString playerPlays;
	CString comPlays;
	CString gameResult;
	CString nowplay;
};

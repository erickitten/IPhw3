
// IPhw3Dlg.h : ���Y��
//

#pragma once
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
#include "afxwin.h"

#include "ImageProcesser.h"

// CIPhw3Dlg ��ܤ��
class CIPhw3Dlg : public CDialogEx
{
// �غc
public:
	CIPhw3Dlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
	enum { IDD = IDD_IPHW3_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
protected:
	HICON m_hIcon;
	ImageProcesser ip;
	cv::VideoCapture vidIn;
	static const UINT ID_TIMER_SECOND = 0x1002;

	// ���ͪ��T�������禡
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

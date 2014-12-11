
// IPhw3Dlg.h : ���Y��
//

#pragma once
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>

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

	double pointDist(cv::Point a,cv::Point b);
	double angle(cv::Point center,cv::Point a,cv::Point b);
	int detectGastureFromBinary(cv::Mat binimg,cv::Mat orgimg = cv::Mat());
	cv::Mat processSampleHist(cv::Mat sample);
	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedLoad();
	CString static_resultstr;
};

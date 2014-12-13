
// IPhw3Dlg.cpp : ��@��
//


#include "stdafx.h"
#include "IPhw3.h"
#include "IPhw3Dlg.h"
#include "afxdialogex.h"
#include <opencv\cv.h>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
#include <cmath>

#include "ImageProcesser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
// CIPhw3Dlg ��ܤ��



CIPhw3Dlg::CIPhw3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIPhw3Dlg::IDD, pParent)
	, static_resultstr(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIPhw3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Text(pDX, STATIC_RESULT, static_resultstr);
	DDX_Text(pDX, STATIC_RESULT, static_resultstr);

	DDX_Control(pDX, DISPLAY_RESIMG, resImgControl);
	DDX_Control(pDX, DISPLAY_BINIMG, binImgControl);
}

BEGIN_MESSAGE_MAP(CIPhw3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_BN_CLICKED(STATIC_LOAD, &CIPhw3Dlg::OnBnClickedLoad)
END_MESSAGE_MAP()


// CIPhw3Dlg �T���B�z�`��

BOOL CIPhw3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �]�w����ܤ�����ϥܡC�����ε{�����D�������O��ܤ���ɡA
	// �ج[�|�۰ʱq�Ʀ��@�~
	SetIcon(m_hIcon, TRUE);			// �]�w�j�ϥ�
	SetIcon(m_hIcon, FALSE);		// �]�w�p�ϥ�

	//AllocConsole();//this only work in debug build
	//FILE* pCout;
	//freopen_s (&pCout,"CONOUT$","w", stdout );//freopen may be unsafe
	cv::Mat samp;
	samp = cv::imread("default_sample_1.jpg", CV_LOAD_IMAGE_COLOR);
	ip.updateSampleHist(samp);
	vidIn.open(0);

	return TRUE;  // �Ǧ^ TRUE�A���D�z�ﱱ��]�w�J�I
}

// �p�G�N�̤p�ƫ��s�[�J�z����ܤ���A�z�ݭn�U�C���{���X�A
// �H�Kø�s�ϥܡC���ϥΤ��/�˵��Ҧ��� MFC ���ε{���A
// �ج[�|�۰ʧ������@�~�C

void CIPhw3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ø�s���˸m���e

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N�ϥܸm����Τ�ݯx��
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �yø�ϥ�
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ��ϥΪ̩즲�̤p�Ƶ����ɡA
// �t�ΩI�s�o�ӥ\����o�����ܡC
HCURSOR CIPhw3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT	CIPhw3Dlg::OnKickIdle(WPARAM,LPARAM)
{
	if(vidIn.isOpened()){
		cv::Mat cam;
		vidIn.read(cam);
		ip.process(cam);
		ImageProcesser::ShowMat(ip.getDetectionImage(),resImgControl);
		ImageProcesser::ShowMat(ip.getBinaryImage(),binImgControl);
	}
	UpdateDialogControls(this,FALSE);
	return	0;
}

void CIPhw3Dlg::OnBnClickedLoad()
{
	cv::Mat src;
	CFileDialog opdig(TRUE, NULL,NULL,OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,_T("All Files (*.*)|*.*||"));


	if (opdig.DoModal() == IDOK){
		CT2A ascii(opdig.GetPathName());//CString -> const char*
		src = cv::imread(ascii.m_psz, CV_LOAD_IMAGE_COLOR);
		ip.process(src);
		//Skin detection ends here
		
		//cv::namedWindow("back projection", CV_WINDOW_NORMAL );
		//cv::imshow("back projection",ip.getBackProjection());
		cv::namedWindow("threshold", CV_WINDOW_NORMAL );
		cv::imshow("threshold",ip.getBinaryImage());
		
		cv::namedWindow("horizontal histogram", CV_WINDOW_NORMAL );
		cv::imshow("horizontal histogram",ip.getHorizontalHistImage());
		cv::namedWindow("vertical histogram", CV_WINDOW_NORMAL );
		cv::imshow("vertical histogram",ip.getHorizontalHistImage());

		cv::namedWindow("detection result", CV_WINDOW_NORMAL );
		cv::imshow("detection result",ip.getDetectionImage());

		static_resultstr = ip.getResultText();		
		UpdateData(FALSE);

		cv::waitKey(0);
		cv::destroyAllWindows();
	}
}


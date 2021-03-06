
// IPhw3Dlg.cpp : 實作檔
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
// CIPhw3Dlg 對話方塊



CIPhw3Dlg::CIPhw3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIPhw3Dlg::IDD, pParent)
	, static_resultstr(_T(""))
	, countDown(0)
	, playerPlays(_T(""))
	, comPlays(_T(""))
	, gameResult(_T(""))
	, nowplay(_T(""))
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
	DDX_Text(pDX, COUNT_DOWN, countDown);
	DDV_MinMaxInt(pDX, countDown, 0, 10);
	DDX_Text(pDX, PL_PLAYS, playerPlays);
	DDX_Text(pDX, COM_PLAYS, comPlays);
	DDX_Text(pDX, GAME_RES, gameResult);
	DDX_Text(pDX, NOWPLAY, nowplay);
}

BEGIN_MESSAGE_MAP(CIPhw3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_BN_CLICKED(STATIC_LOAD, &CIPhw3Dlg::OnBnClickedLoad)
	ON_BN_CLICKED(GAMEBTN, &CIPhw3Dlg::OnBnClickedGamebtn)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CIPhw3Dlg 訊息處理常式

BOOL CIPhw3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	vidIn.open(0);
	srand (time(NULL));

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CIPhw3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CIPhw3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT	CIPhw3Dlg::OnKickIdle(WPARAM,LPARAM)
{
	if(vidIn.isOpened()){
		cv::Mat cam;
		vidIn.read(cam);
		if(!cam.empty()){
			ip.process(cam);
			ImageProcesser::ShowMat(ip.getDetectionImage(),resImgControl);
			ImageProcesser::ShowMat(ip.getBinaryImage(),binImgControl);
			nowplay =  ip.getResultText();
		}else{
			nowplay = _T("waiting cam...");
		}
	}
	UpdateDialogControls(this,FALSE);
	UpdateData(FALSE);

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



void CIPhw3Dlg::OnBnClickedGamebtn()
{
	countDown = 3;
	SetTimer(ID_TIMER_SECOND,1000, NULL);
	UpdateData(FALSE);
}


void CIPhw3Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if(countDown > 0){
		countDown--;
		UpdateData(FALSE);
	}else{
		int pla = ip.getResultInt();
		int com = rand() % 3;

		playerPlays = ip.getResultText();
		if(com == 0){
			comPlays = _T("paper");
		}else if(com == 1){
			comPlays = _T("scissor");
		}else{
			comPlays = _T("stone");
		}

		if(pla < 0){
			gameResult =  _T("unknown ");
		}else if(pla == com){
			gameResult =  _T("draw");
		}else if(pla - com == 1 || (pla == 0 && com ==2)){
			gameResult =  _T("you win! :)");
		}else{
			gameResult =  _T("you lose :(");
		}
		KillTimer(ID_TIMER_SECOND);
		UpdateData(FALSE);
	}

	CDialogEx::OnTimer(nIDEvent);
}

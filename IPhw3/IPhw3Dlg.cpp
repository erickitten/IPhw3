
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
// CIPhw3Dlg 對話方塊



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
}

BEGIN_MESSAGE_MAP(CIPhw3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(STATIC_LOAD, &CIPhw3Dlg::OnBnClickedLoad)
END_MESSAGE_MAP()


// CIPhw3Dlg 訊息處理常式

BOOL CIPhw3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	//AllocConsole();//this only work in debug build
	//FILE* pCout;
	//freopen_s (&pCout,"CONOUT$","w", stdout );//freopen may be unsafe

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

	float hue_range[] = { 0, 180 };
	float sal_range[] = { 0, 255 };
	const float* ranges[] = {hue_range,sal_range};
	int histSize[] = {20,20};
	int channels[] = { 0, 1 };

//distence bewteen 2 points
double CIPhw3Dlg::pointDist(cv::Point a,cv::Point b)  
{  
	return sqrt( (double)((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)) );  
}

double CIPhw3Dlg::angle(cv::Point center,cv::Point a,cv::Point b)
{
	double dist1 = pointDist(a,center);
	double dist2 = pointDist(b,center);

	double dot = (a.x - center.x) * (b.x - center.x) + (a.y - center.y) * (b.y - center.y);
	return (acos(dot / (dist1 * dist2))  * 180.0) / M_PI;
}

/**
tries to detect gesture using contours
input must be a binary image
if the original image is given ,dectection result will be draw on it
*/
int CIPhw3Dlg::detectGastureFromBinary(cv::Mat binimg,cv::Mat orgimg)
{
	/*
	oh my god this finally worked!!!
	the reason that it failed assert(_pFirstBlock == pHead)is the memory of the contours
	is allocated by opencv dlls (findContours) ,but free in main program (alloc by A free by B)
	crossing .dll boundary and therefore causing memory violations
	change  MSVC Runtime library from "Multi-threaded Debug (/MTd)" to "Multi-threaded Debug DLL (/MDd)" helps
		(setting -> c/c++ -> code generation -> RuntimeLibrary)
	for others ,ref to https://stackoverflow.com/questions/18882760/debug-assertion-failed-expression-pfirstblock-phead
	*/

	vector<vector<cv::Point>> contours;
	cv::findContours(binimg,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
		
	//find the contour with maximum area
	double maxarea = 0,area = 0;
	int maxi = 0;
	for(int i=0;i<contours.size();i++){
		area = cv::contourArea(contours.at(i));
		if(maxarea < area){
			maxi = i;
			maxarea = area;
		}
	}	
	
	vector<cv::Point> maxitem = contours.at(maxi);
	vector<int>hull;
	vector<cv::Vec4i>defect;
	cv::convexHull(maxitem,hull);
	
	cv::convexityDefects(maxitem,hull,defect);
	//determine sigificance (if it counts as hole bewteen finger)
	vector<cv::Vec4i>sigificantDefect;
	vector<double>dectectionAngle;
	for(int i=0;i<defect.size();i++){
		double defectArea = (pointDist(maxitem.at(defect.at(i)[0]),maxitem.at(defect.at(i)[1])) * (double)(defect.at(i)[3] >> 8)) /2;
		double defectAngle = angle(maxitem.at(defect.at(i)[2]),maxitem.at(defect.at(i)[0]),maxitem.at(defect.at(i)[1]));

		if(defectArea > maxarea * 0.05 && defectAngle < 90){
			sigificantDefect.push_back(defect.at(i));
			//dectectionAngle.push_back(defectAngle);
		}
	}


	if(!orgimg.empty() && binimg.size() == orgimg.size()){
		drawContours(orgimg,contours,maxi,cv::Scalar(255,0,0));
		for(int i=1;i<hull.size();i++){//draw hull
			cv::line(orgimg,maxitem.at(hull.at(i-1)),maxitem.at(hull.at(i)),cv::Scalar(0,255,0),2);
		}
		for(int i=0;i<sigificantDefect.size();i++){
			cv::line(orgimg,maxitem.at(sigificantDefect.at(i)[0]),maxitem.at(sigificantDefect.at(i)[2]),cv::Scalar(0,0,255),2);
			cv::line(orgimg,maxitem.at(sigificantDefect.at(i)[1]),maxitem.at(sigificantDefect.at(i)[2]),cv::Scalar(0,0,255),2);
			//if(dectectionAngle.at(i)<95){
				cv::circle(orgimg,maxitem.at(sigificantDefect.at(i)[2]),5,cv::Scalar(0,255,255),-1);
			//}else{
				//cv::circle(orgimg,maxitem.at(sigificantDefect.at(i)[2]),5,cv::Scalar(255,255,0),-1);
			//}
		}
	}

	return (int)sigificantDefect.size();
}

cv::Mat CIPhw3Dlg::processSampleHist(cv::Mat sample)
{
	cv::Mat hsv,hist;
	//convert to (Hue, Saturation, Value) space
	//ref: https://zh.wikipedia.org/wiki/HSL%E5%92%8CHSV%E8%89%B2%E5%BD%A9%E7%A9%BA%E9%97%B4
	cv::cvtColor( sample, hsv, CV_BGR2HSV );

	cv::calcHist(&hsv, 1, channels, cv::Mat(), hist, 2, histSize, ranges, true, false );
	normalize( hist, hist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat() );
	return hist;
}


void CIPhw3Dlg::OnBnClickedLoad()
{
	cv::Mat src,hsv,samp,backproj,binimg,vph,hph,vphimg,hphimg;
	CFileDialog opdig(TRUE, NULL,NULL,OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,_T("All Files (*.*)|*.*||"));

	if (opdig.DoModal() == IDOK){
		CT2A ascii(opdig.GetPathName());//CString -> const char*
		src = cv::imread(ascii.m_psz, CV_LOAD_IMAGE_COLOR);
		samp = cv::imread("default_sample_1.jpg", CV_LOAD_IMAGE_COLOR);
		cv::cvtColor( src, hsv, CV_BGR2HSV );
		cv::calcBackProject( &hsv, 1, channels,processSampleHist(samp), backproj, ranges, 1, true );

		cv::threshold(backproj, binimg, 3, 255, CV_THRESH_BINARY /*| CV_THRESH_OTSU*/);
		//must use floating point Mat at output
		cv::reduce(binimg,hph,0,CV_REDUCE_SUM,CV_64FC1);
		cv::reduce(binimg,vph,1,CV_REDUCE_SUM,CV_64FC1);
				
		vphimg.create(binimg.size(),binimg.depth());
		vphimg.setTo(cv::Scalar(0));//set to black
		hphimg.create(binimg.size(),binimg.depth());
		hphimg.setTo(cv::Scalar(0));

		//picture size problem
		for(int i=0;i<binimg.size().width;i++){
			cv::line( hphimg,cv::Point(i,binimg.size().height),cv::Point(i,binimg.size().height - (int)hph.at<double>(0,i)/(2*255)),CV_RGB(255,255,255));
		}
		for(int i=0;i<binimg.size().height;i++){
			cv::line( vphimg,cv::Point(0,i),cv::Point((int)vph.at<double>(i,0)/(2*255),i),CV_RGB(255,255,255));
		}
		
		//Skin detection ends here
		
		cv::namedWindow("back projection", CV_WINDOW_NORMAL );
		cv::imshow("back projection",backproj);
		cv::namedWindow("threshold", CV_WINDOW_NORMAL );
		cv::imshow("threshold",binimg);
		
		cv::namedWindow("horizontal histogram", CV_WINDOW_NORMAL );
		cv::imshow("horizontal histogram",hphimg);
		cv::namedWindow("vertical histogram", CV_WINDOW_NORMAL );
		cv::imshow("vertical histogram",vphimg);


		int defect = detectGastureFromBinary(binimg,src);
		cv::namedWindow("detection result", CV_WINDOW_NORMAL );
		cv::imshow("detection result",src);

		if(defect == 0){
			static_resultstr = _T("stone");
		}else if(defect == 1){
			static_resultstr = _T("scissor");
		}else if(defect == 4){
			static_resultstr = _T("paper");
		}else{
			static_resultstr = _T("undefined");
		}
		UpdateData(FALSE);

		cv::waitKey(0);
		cv::destroyAllWindows();
	}
}

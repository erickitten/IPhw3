#include "stdafx.h"
#include "ImageProcesser.h"
#include <opencv\cv.h>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
//#include <opencv2\photo\photo.hpp>
#include <cmath>


using namespace std;

//use default params
ImageProcesser::ImageProcesser(void)
{
	significanceDefectRatio = 0.05;
	significanceDefectAngle = 90;
	lowHue = 0,highHue = 25;
	lowSaturation = 45,highSaturation = 190;
}

//static functions & variables

double ImageProcesser::pointDist(cv::Point a,cv::Point b)  
{  
	return sqrt( (double)((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)) );  
}

double ImageProcesser::angle(cv::Point center,cv::Point a,cv::Point b)
{
	double dist1 = pointDist(a,center);
	double dist2 = pointDist(b,center);

	double dot = (a.x - center.x) * (b.x - center.x) + (a.y - center.y) * (b.y - center.y);
	return (acos(dot / (dist1 * dist2))  * 180.0) / M_PI;
}

//show the Mat on picture control
void ImageProcesser::ShowMat( cv::Mat m_matCVImg,CStatic &m_staticImage)  
{
	/*
	modified from http://kvy.com.ua/transformation-of-opencv-image-to-mfc-image-in-msvc-project/
	*/
	cv::Size m_sizeShow;
	CImage* m_pImg =NULL;

	//1. Define MFC window size:
	RECT r;
	m_staticImage.GetClientRect(&r);
	m_sizeShow = cv::Size(r.right, r.bottom);
	//do DWORD alignment (cvMat only have 4-byte alignment ,there size must be dividiable by 4)
	m_sizeShow.height = (m_sizeShow.height %4 ==0)?m_sizeShow.height:m_sizeShow.height - (m_sizeShow.height %4);
	m_sizeShow.width = (m_sizeShow.width %4 ==0)?m_sizeShow.width:m_sizeShow.width - (m_sizeShow.width %4);
 
	//2. The size of m_matCVImg is not always the same as an MFC window¡¦s:
	cv::Mat matImgTmp;
	if (m_matCVImg.size() != m_sizeShow)
	{
		matImgTmp = cv::Mat(m_sizeShow, CV_8UC3);
		cv::resize(m_matCVImg, matImgTmp, m_sizeShow, 0, 0, cv::INTER_AREA);
	} else {
		matImgTmp = m_matCVImg.clone();
	}
 
	//3. Rotate the image:
	cv::flip(matImgTmp, matImgTmp, 0);
 
	//4. Create an MFC image:
	m_pImg = new CImage;
	m_pImg->Create(m_sizeShow.width, m_sizeShow.height, 24);
 
	//5. For m_pImg you need a header. Create it by using BITMAPINFO structure
	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = 24;
	bitInfo.bmiHeader.biWidth = m_sizeShow.width;
	bitInfo.bmiHeader.biHeight = m_sizeShow.height;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant =
		bitInfo.bmiHeader.biClrUsed =
		bitInfo.bmiHeader.biSizeImage =
		bitInfo.bmiHeader.biXPelsPerMeter =
		bitInfo.bmiHeader.biYPelsPerMeter = 0;
 
	//6. Add header and OpenCV image¡¦s data to m_pImg
	StretchDIBits(m_pImg->GetDC(), 0, 0,
    m_sizeShow.width, m_sizeShow.height, 0, 0,
    m_sizeShow.width, m_sizeShow.height,
    matImgTmp.data, &bitInfo, DIB_RGB_COLORS, SRCCOPY);
 
	//7. Display m_pImg in MFC window
	m_pImg->BitBlt(::GetDC(m_staticImage.m_hWnd), 0, 0);
 
	//8. (Optional) Release m_pImg, if you will not use it:
	if (m_pImg)
	{
		m_pImg->ReleaseDC();
		delete m_pImg;
		m_pImg = NULL;
	}
}


int ImageProcesser::detectGastureFromBinary(cv::Mat binimg,cv::Mat orgimg)
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
	vector<cv::Point> maxitem;
	if(!contours.empty() && contours.at(maxi).size() > 500){
		maxitem = contours.at(maxi);
	}else{//no contour found		
		return -1;
	}
	vector<int>hull;
	vector<cv::Vec4i>defect;
	cv::convexHull(maxitem,hull);
	
	cv::convexityDefects(maxitem,hull,defect);

	//determine sigificance (if it counts as hole bewteen finger)
	//by relative size / angle / direction
	vector<cv::Vec4i> sigificantDefect;
	for(int i=0;i<defect.size();i++){
		double defectArea = (pointDist(maxitem.at(defect.at(i)[0]),maxitem.at(defect.at(i)[1])) * (double)(defect.at(i)[3] >> 8)) /2;
		double defectAngle = angle(maxitem.at(defect.at(i)[2]),maxitem.at(defect.at(i)[0]),maxitem.at(defect.at(i)[1]));
		int upward = maxitem.at(defect.at(i)[2]).y - (maxitem.at(defect.at(i)[0]).y + maxitem.at(defect.at(i)[1]).y) /2;

		if(defectArea > maxarea * significanceDefectRatio && defectAngle < significanceDefectAngle && upward >=0){
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
			//if(dectectionAngle.at(i)<significanceDefectAngle){
			cv::circle(orgimg,maxitem.at(sigificantDefect.at(i)[2]),5,cv::Scalar(0,255,255),-1);
			//}else{
			//cv::circle(orgimg,maxitem.at(sigificantDefect.at(i)[2]),5,cv::Scalar(255,255,0),-1);
			//}
		}
	}
	return (int)sigificantDefect.size();
}



//clear current image and its related information
void ImageProcesser::clearCurrent(void)
{
	currentImage.release();
	binaryImage.release();
	verticalHistImage.release();
	horizontalHistImage.release();
}


void ImageProcesser::process(cv::Mat in)
{
	cv::Mat blur,hsv,hph,vph;

	//clean out previous data
	this->clearCurrent();

	currentImage = in.clone();
	//cv::bilateralFilter(in,currentImage,5,250,250);	//deal with camera noise
	cv::cvtColor( currentImage, hsv, CV_BGR2HSV );
	
	cv::inRange( hsv,cv::Scalar(lowHue,lowSaturation,10,0), cv::Scalar(highHue,highSaturation,255,0),binaryImage);
	
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(9,9));
	cv::morphologyEx(binaryImage,binaryImage,cv::MORPH_CLOSE,kernel);
	cv::morphologyEx(binaryImage,binaryImage,cv::MORPH_OPEN,kernel);


	//must use floating point Mat at output
	cv::reduce(binaryImage,hph,0,CV_REDUCE_SUM,CV_64FC1);
	cv::reduce(binaryImage,vph,1,CV_REDUCE_SUM,CV_64FC1);
				
	verticalHistImage.create(binaryImage.size(),binaryImage.depth());
	verticalHistImage.setTo(cv::Scalar(0));//set to black
	horizontalHistImage.create(binaryImage.size(),binaryImage.depth());
	horizontalHistImage.setTo(cv::Scalar(0));

	//picture size problem
	for(int i=0;i<binaryImage.size().width;i++){
		cv::line( horizontalHistImage,cv::Point(i,binaryImage.size().height),cv::Point(i,binaryImage.size().height - (int)hph.at<double>(0,i)/(2*255)),CV_RGB(255,255,255));
	}
	for(int i=0;i<binaryImage.size().height;i++){
		cv::line( verticalHistImage,cv::Point(0,i),cv::Point((int)vph.at<double>(i,0)/(2*255),i),CV_RGB(255,255,255));
	}

	detectionImage = currentImage.clone();
	numOfDefect = detectGastureFromBinary(binaryImage.clone(),detectionImage);
}

DO_IMG_GETTER(currentImage,getCurrentImage)
DO_IMG_GETTER_CVT(binaryImage,getBinaryImage)
DO_IMG_GETTER_CVT(verticalHistImage,getVerticalHistImage)
DO_IMG_GETTER_CVT(horizontalHistImage,getHorizontalHistImage)
DO_IMG_GETTER(detectionImage,getDetectionImage)

int ImageProcesser::geNnumOfDefect()
{
	return numOfDefect;
}


CString ImageProcesser::getResultText()
{
	if(numOfDefect ==0){
		return _T("stone");
	}
	if(numOfDefect >0 && numOfDefect <4){
		return _T("scissor");
	}
	if(numOfDefect >=4){
		return _T("paper");
	}
	return _T("undefined");
}

int ImageProcesser::getResultInt()
{
	if(numOfDefect ==0){
		return 2;
	}
	if(numOfDefect >0 && numOfDefect <4){
		return 1;
	}
	if(numOfDefect >=4){
		return 0;
	}
	return -1;
}

ImageProcesser::~ImageProcesser(void)
{
}



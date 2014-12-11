#include "stdafx.h"
#include "ImageProcesser.h"
#include <opencv\cv.h>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
#include <cmath>

using namespace std;

//use default params
ImageProcesser::ImageProcesser(void)
{
	hue_range[0] = 0; hue_range[1] = 180;
	sal_range[0]= 0; sal_range[1] = 255;
	ranges[0] = hue_range; ranges[1] = sal_range;
	histSize[0] = 20; histSize[1] = 20;
	channels[0] = 0; channels[1] = 1;
	significanceDefectRatio = 0.05;
	significanceDefectAngle = 90;
}

//static functions

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

		if(defectArea > maxarea * significanceDefectRatio && defectAngle < significanceDefectAngle){
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


//clear current image and its related information
void ImageProcesser::clearCurrent(void)
{
	currentImage.release();
	backProjection.release();
	binaryImage.release();
	verticalHistImage.release();
	horizontalHistImage.release();
}



void ImageProcesser::process(cv::Mat in)
{
	cv::Mat hsv,hph,vph;

	//clean out previous data
	this->clearCurrent();

	currentImage = in;
	cv::cvtColor( currentImage, hsv, CV_BGR2HSV );
	cv::calcBackProject( &hsv, 1, channels,sampleHist, backProjection, ranges, 1.0, true );

	cv::threshold(backProjection, binaryImage, 3, 255, CV_THRESH_BINARY /*| CV_THRESH_OTSU*/);
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

}


ImageProcesser::~ImageProcesser(void)
{
}

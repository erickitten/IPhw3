#pragma once
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>



#define DO_IMG_GETTER(name,getName) \
	cv::Mat ImageProcesser:: ## getName ## () \
	{ \
	return name ## .clone(); \
	}

#define DO_IMG_GETTER_CVT(name,getName) \
	cv::Mat ImageProcesser:: ## getName ## () \
	{ \
	cv::Mat cvt; \
	cv::cvtColor( name, cvt, CV_GRAY2BGR );\
	return cvt; \
	}

#define DEF_IMG_GETTER(getName)  \
	cv::Mat getName ## ()

class ImageProcesser
{
protected:
	
	//helper functions
	static double pointDist(cv::Point a,cv::Point b);
	static double angle(cv::Point center,cv::Point a,cv::Point b);
	
	//image processing params
	double significanceDefectRatio;
	double significanceDefectAngle;	

	
	cv::Mat currentImage;
	cv::Mat binaryImage;
	cv::Mat verticalHistImage;
	cv::Mat horizontalHistImage;
	cv::Mat detectionImage;
	int numOfDefect;

	int detectGastureFromBinary(cv::Mat binimg,cv::Mat orgimg);
public:
	static void ShowMat( cv::Mat m_matCVImg,CStatic &m_staticImage);

	ImageProcesser(void);
	void clearCurrent();
	void process(cv::Mat in);

	DEF_IMG_GETTER(getCurrentImage);
	DEF_IMG_GETTER(getBackProjection);
	DEF_IMG_GETTER(getBinaryImage);
	DEF_IMG_GETTER(getVerticalHistImage);
	DEF_IMG_GETTER(getHorizontalHistImage);
	DEF_IMG_GETTER(getDetectionImage);
	int geNnumOfDefect();
	CString getResultText();
	int getResultInt();

	~ImageProcesser(void);
};


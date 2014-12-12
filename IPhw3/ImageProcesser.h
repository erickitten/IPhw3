#pragma once
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>

#define DO_IMG_GETTER(name,getName) \
	cv::Mat ImageProcesser:: ## getName ## () \
	{ \
	return name ## .clone(); \
	}

#define DEF_IMG_GETTER(getName)  \
	cv::Mat getName ## ()

class ImageProcesser
{
protected:
	static float hue_range[2];
	static float sal_range[2];
	static const float* ranges[2];
	
	static double pointDist(cv::Point a,cv::Point b);
	static double angle(cv::Point center,cv::Point a,cv::Point b);
	
	//image processing params
	int histSize[2];
	int channels[2];
	double significanceDefectRatio;
	double significanceDefectAngle;	
	cv::Mat sampleHist;
	
	cv::Mat currentImage;
	cv::Mat backProjection;
	cv::Mat binaryImage;
	cv::Mat verticalHistImage;
	cv::Mat horizontalHistImage;
	cv::Mat detectionImage;
	int numOfDefect;

	int detectGastureFromBinary(cv::Mat binimg,cv::Mat orgimg);
public:


	ImageProcesser(void);
	void clearCurrent();
	void process(cv::Mat in);
	void updateSampleHist(cv::Mat sample);

	void getAllImages(cv::Mat *org,cv::Mat *backProjection,cv::Mat *binary,cv::Mat *verticalHist,cv::Mat *horizontalHist,cv::Mat *detection);
	DEF_IMG_GETTER(getCurrentImage);
	DEF_IMG_GETTER(getBackProjection);
	DEF_IMG_GETTER(getBinaryImage);
	DEF_IMG_GETTER(getVerticalHistImage);
	DEF_IMG_GETTER(getHorizontalHistImage);
	DEF_IMG_GETTER(getDetectionImage);
	int geNnumOfDefect();
	CString getResultText();

	~ImageProcesser(void);
};


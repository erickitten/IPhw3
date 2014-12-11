#pragma once
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>


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

	CString getResultText();
	
	~ImageProcesser(void);
};


//#include <climits>
//#include <cstdlib>
//#include <iostream>
//#include <vector>
//
#include "tbb/pipeline.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_reduce.h"
#include "tbb/tick_count.h"

#include <math.h>

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <iostream>

using namespace cv;

Mat negative(Mat a_source)
{
	Mat _retImage;
	bitwise_not(a_source, _retImage);
	return _retImage;
}

Mat edgeDetection(Mat a_source)
{
	Mat _gx = (Mat_<double>(3,3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
	Mat _gy = (Mat_<double>(3,3) << -1, -1, -1, 0, 0, 0, 1, 1, 1);
	Mat _gxI, _gyI, _retImage, _gxIF, _gyIF, _retImageF;
	filter2D(a_source, _gxI, -1, _gx);
	filter2D(a_source, _gyI, -1, _gy);
	_gxI.convertTo(_gxIF, CV_64FC3);
	_gyI.convertTo(_gyIF, CV_64FC3);
	magnitude(_gxIF, _gyIF, _retImageF);
	_retImageF.convertTo(_retImage, CV_8UC3);
	return _retImage;
}

class ImageInputFilter: public tbb::filter
{
public:
	ImageInputFilter( VideoCapture* a_capture ) : filter(serial_in_order), capture(a_capture), isVideo(true) {}
	ImageInputFilter( Mat* a_image ) : filter(serial_in_order), frame(*a_image), isVideo(false) {}
    ~ImageInputFilter();
private:
    VideoCapture* capture;
	Mat frame;
    bool isVideo;
    /*override*/ void* operator()(void*)
	{
		if (isVideo)
		{
			*capture>>frame;
			return &frame;
		} else
		{
			return &frame;
		}
	}
};
 
//! Filter that changes each decimal number to its square.
class NegativeFilter: public tbb::filter
{
public:
	NegativeFilter() : tbb::filter(parallel) {};
    /*override*/void* operator()( void* item )
	{
		Mat& input = *static_cast<Mat*>(item);
		return &negative(input);
	}
};

//! Filter that changes each decimal number to its square.
class EdgeDetectionFilter: public tbb::filter
{
public:
	EdgeDetectionFilter() : tbb::filter(parallel) {};
    /*override*/void* operator()( void* item )
	{
		Mat& input = *static_cast<Mat*>(item);
		return &edgeDetection(input);
	}
};
     
//! Filter that writes each buffer to a file.
class ImageOutputFilter: public tbb::filter
{
public:
	std::string target;

	ImageOutputFilter(std::string a_target) : tbb::filter(serial_in_order), target(a_target) {};
    /*override*/void* operator()( void* item )
	{
		Mat& input = *static_cast<Mat*>(item);
		imshow(target, input);
		return NULL;
	}
};

int main( int argc, char** argv )
{
	// ������������� ������������ ����� TBB
	tbb::task_scheduler_init init;

	/// Read image given by user
	//Mat image = imread( argv[1] );
	Mat image = imread( "touhou-Touhou-Project-anime-Hakurei-Reimu.33p.jpg" );
	Mat new_image, new_image2;

	/// Do the operation new_image(i,j) = 255 - image(i,j)
	new_image=negative(image);

	/// Do the operation new_image(i,j) = alpha*image(i,j) + beta
	new_image2=edgeDetection(image);

	/// Create Windows
	namedWindow("Original Image", 1);
	namedWindow("New Image", 1);
	namedWindow("New Image #2", 1);

	/// Show stuff
	imshow("Original Image", image);
	imshow("New Image", new_image);
	imshow("New Image #2", new_image2);
	imwrite("negative.jpg", new_image);
	imwrite("edges.jpg", new_image2);

	/// Wait until user press some key
	waitKey();
	return 0;
}

//int main( int argc, char** argv )
//{
//	//VideoCapture cap(0);
//    VideoCapture cap("D:\\Cool\\Diablo ������\\Demon_Hunter_RURU.mpg"); // open the default camera
//	//VideoCapture cap("E:\\Films\\Tengen Toppa Gurren-Lagann\\��� 3 [tapochek.net]\\DVD 7 [tapochek.net]\\VIDEO_TS\\VTS_01_1.VOB");
//    if(!cap.isOpened())  // check if we succeeded
//        return -1;
//
//    Mat edges;
//    namedWindow("edges",1);
//    for(;;)
//    {
//        Mat frame;
//        cap >> frame; // get a new frame from camera
//		
//        edges=negative(frame);
//        imshow("edges", edges);
//        if(waitKey(1) >= 0) break;
//    }
//    // the camera will be deinitialized automatically in VideoCapture destructor
//    return 0;
//}
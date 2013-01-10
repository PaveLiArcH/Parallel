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

const int numTokens=4;
const bool g_isShow=true;

Mat* negative(Mat* a_source)
{
	Mat* _retImage=new Mat();
	bitwise_not(*a_source, *_retImage);
	return _retImage;
}

Mat* edgeDetection(Mat* a_source)
{
	Mat *_retImage=new Mat();
	Mat _gx = (Mat_<double>(3,3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
	Mat _gy = (Mat_<double>(3,3) << -1, -1, -1, 0, 0, 0, 1, 1, 1);
	Mat _gxI, _gyI, _gxIF, _gyIF, _retImageF;
	filter2D(*a_source, _gxI, -1, _gx);
	filter2D(*a_source, _gyI, -1, _gy);
	_gxI.convertTo(_gxIF, CV_64FC3);
	_gyI.convertTo(_gyIF, CV_64FC3);
	magnitude(_gxIF, _gyIF, _retImageF);
	_retImageF.convertTo(*_retImage, CV_8UC3);
	return _retImage;
}

class ImageInputFilter: public tbb::filter
{
public:
	ImageInputFilter( VideoCapture* a_capture, bool a_show ) : filter(serial_in_order), capture(a_capture), isVideo(true), isShow(a_show) {}
	ImageInputFilter( Mat* a_image, bool a_show ) : filter(serial_in_order), frame(a_image), isVideo(false), isShow(a_show) {}
private:
    VideoCapture* capture;
	Mat* frame;
    bool isVideo, isShow;
    /*override*/ void* operator()(void*)
	{
		if (isVideo)
		{
			Mat* _retMat=new Mat();
			if (!capture->grab())
			{
				delete _retMat;
				return NULL;
			} else
			{
				capture->retrieve(*_retMat);
				frame=_retMat;
				if (isShow && frame)
				{
					imshow("Source", *frame);
				}
				return _retMat;
			}
		} else
		{
			if (frame)
			{
				Mat* _retMat=new Mat(*frame);
				//_retMat=frame;
				if (isShow)
				{
					imshow("Source", *frame);
				}
				frame=NULL;
				return _retMat;
			} else
			{
				return NULL;
			}
		}
	}
};
 
//! Filter that inverses colors of given image.
class NegativeFilter: public tbb::filter
{
public:
	NegativeFilter() : tbb::filter(parallel) {};
    /*override*/void* operator()( void* item )
	{
		Mat* input = static_cast<Mat*>(item);
		Mat* _out=negative(input);
		delete input;
		return _out;
	}
};

//! Filter that calls edge detection function on given image.
class EdgeDetectionFilter: public tbb::filter
{
public:
	EdgeDetectionFilter() : tbb::filter(parallel) {};
    /*override*/void* operator()( void* item )
	{
		Mat* input = static_cast<Mat*>(item);
		Mat* _out=edgeDetection(input);
		delete input;
		return _out;
	}
};
     
//! Filter that writes each buffer to a file.
class ImageOutputFilter: public tbb::filter
{
public:
	std::string target;
	bool isShow;

	ImageOutputFilter(std::string a_target, bool a_show) : tbb::filter(serial_in_order), target(a_target), isShow(a_show) {};
    /*override*/void* operator()( void* item )
	{
		Mat* input = static_cast<Mat*>(item);
		if (isShow)
		{
			imshow(target, *input);
		}
		delete input;
		if (isShow)
		{
			waitKey(1);
		}
		return NULL;
	}
};

int main( int argc, char** argv )
{
	// инициализация планировщика задач TBB
	tbb::task_scheduler_init init;

	/// Create Windows
	if (g_isShow)
	{
		namedWindow("Source", 1);
		namedWindow("New Image", 1);
	}

	/// Read image given by user
	//Mat image = imread( argv[1] );
	Mat image = imread( "touhou-Touhou-Project-anime-Hakurei-Reimu.33p.jpg" );

	//VideoCapture cap(0);
	VideoCapture cap("D:\\Docs\\Visual Studio 2010\\Projects\\Parallel\\Red Bull Stratos  Jump (1080p).mp4");
    if(!cap.isOpened())  // check if we succeeded
        return -1;
	
	Mat new_image, new_image2;

	ImageInputFilter _inFilter(&cap, g_isShow);
	NegativeFilter _negativeFilter;
	EdgeDetectionFilter _edgeFilter;
	ImageOutputFilter _outFilter("New Image", g_isShow);
	// Create filters sequence when parallel_pipeline() is being run
    //tbb::parallel_pipeline( 8, _inFilter & _negativeFilter & _outFilter );

	// Create the pipeline
    tbb::pipeline pipeline;

    pipeline.add_filter( _inFilter );
	pipeline.add_filter( _edgeFilter );
	pipeline.add_filter( _negativeFilter );
    pipeline.add_filter( _outFilter );

	tbb::tick_count t0 = tbb::tick_count::now();
	pipeline.run( 8 );
	tbb::tick_count t1 = tbb::tick_count::now();

	std::cout << "Time for action =" << (t1-t0).seconds() << " seconds. Tokens: " << numTokens << std::endl;

	/// Wait until user press some key
	waitKey();
	return 0;
}
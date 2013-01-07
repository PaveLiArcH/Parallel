//#include <climits>
//#include <cstdlib>
//#include <iostream>
//#include <vector>
//
//#include "tbb/task_scheduler_init.h"
//#include "tbb/blocked_range.h"
//#include "tbb/parallel_reduce.h"
//#include "tbb/tick_count.h"
//
//using namespace std;
//using namespace tbb;
//
//int main(int argc, char* argv[])
//{
//	// инициализация планировщика задач TBB
//	task_scheduler_init init;
//
//	
//
//	return 0;
//}
//

#include <math.h>

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <iostream>

using namespace cv;

double alpha; /**< Simple contrast control */
int beta;  /**< Simple brightness control */

Mat negative(Mat a_source)
{
	Mat _retImage = Mat::zeros( a_source.size(), a_source.type() );
	int _rows=a_source.rows, _cols=a_source.cols, _channels=a_source.channels();
	for( int y = 0; y < _rows; y++ )
	{
		for( int x = 0; x < _cols; x++ )
		{
			for( int c = 0; c < _channels; c++ )
			{
				_retImage.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( 255-a_source.at<Vec3b>(y,x)[c] );
			}
		}
	}
	return _retImage;
}

inline uchar getPixel(Mat image, int c, int y, int x, int rows, int cols)
{
	return (y>=0)&&(y<rows)&&(x>=0)&&(x<cols)?image.at<Vec3b>(y, x)[c]:0;
}

Mat edgeDetection(Mat a_source)
{
	Mat _retImage = Mat::zeros( a_source.size(), a_source.type() );
	int _rows=a_source.rows, _cols=a_source.cols, _channels=a_source.channels();
	for( int y = 0; y < _rows; y++ )
	{
		for( int x = 0; x < _cols; x++ )
		{

			for( int c = 0; c < _channels; c++ )
			{
				/// Prewitt
				double _gx=-getPixel(a_source, c, y-1, x-1, _rows, _cols)-getPixel(a_source, c, y, x-1, _rows, _cols)-getPixel(a_source, c, y+1, x-1, _rows, _cols);
				_gx+=getPixel(a_source, c, y-1, x+1, _rows, _cols)+getPixel(a_source, c, y, x+1, _rows, _cols)+getPixel(a_source, c, y+1, x+1, _rows, _cols);

				double _gy=-getPixel(a_source, c, y-1, x-1, _rows, _cols)-getPixel(a_source, c, y-1, x, _rows, _cols)-getPixel(a_source, c, y-1, x+1, _rows, _cols);
				_gy+=getPixel(a_source, c, y+1, x-1, _rows, _cols)+getPixel(a_source, c, y+1, x, _rows, _cols)+getPixel(a_source, c, y+1, x+1, _rows, _cols);

				/// Sobel
				/*double _gx=-getPixel(a_source, c, y-1, x-1, _rows, _cols)-2*getPixel(a_source, c, y, x-1, _rows, _cols)-getPixel(a_source, c, y+1, x-1, _rows, _cols);
				_gx+=getPixel(a_source, c, y-1, x+1, _rows, _cols)+2*getPixel(a_source, c, y, x+1, _rows, _cols)+getPixel(a_source, c, y+1, x+1, _rows, _cols);

				double _gy=-getPixel(a_source, c, y-1, x-1, _rows, _cols)-2*getPixel(a_source, c, y-1, x, _rows, _cols)-getPixel(a_source, c, y-1, x+1, _rows, _cols);
				_gy+=getPixel(a_source, c, y+1, x-1, _rows, _cols)+2*getPixel(a_source, c, y+1, x, _rows, _cols)+getPixel(a_source, c, y+1, x+1, _rows, _cols);*/

				/// Scharr
				/*double _gx=-3*getPixel(a_source, c, y-1, x-1, _rows, _cols)-10*getPixel(a_source, c, y, x-1, _rows, _cols)-3*getPixel(a_source, c, y+1, x-1, _rows, _cols);
				_gx+=3*getPixel(a_source, c, y-1, x+1, _rows, _cols)+10*getPixel(a_source, c, y, x+1, _rows, _cols)+3*getPixel(a_source, c, y+1, x+1, _rows, _cols);

				double _gy=-3*getPixel(a_source, c, y-1, x-1, _rows, _cols)-10*getPixel(a_source, c, y-1, x, _rows, _cols)-3*getPixel(a_source, c, y-1, x+1, _rows, _cols);
				_gy+=3*getPixel(a_source, c, y+1, x-1, _rows, _cols)+10*getPixel(a_source, c, y+1, x, _rows, _cols)+3*getPixel(a_source, c, y+1, x+1, _rows, _cols);*/

				_retImage.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( sqrt(_gx*_gx+_gy*_gy));
			}
		}
	}
	return _retImage;
}

//Mat edgeDetection(Mat a_source)
//{
//	Mat _dxM(3, 3, CV_32SC3, Scalar(1, 1, 1, 1));
//	_dxM.col(0).setTo(_dxM.col(0)*-1);
//	_dxM.col(1).setTo(_dxM.col(1)*0);
//
//	Mat _dyM(3, 3, CV_32SC3, Scalar(1, 1, 1, 1));
//	_dyM.row(0).setTo(_dxM.row(0)*-1);
//	_dyM.row(1).setTo(_dxM.row(1)*0);
//
//	Mat _retImage = Mat::zeros( a_source.size(), a_source.type() );
//	int _rows=a_source.rows+1, _cols=a_source.cols+1, _channels=a_source.channels();
//
//	Mat _expandedImage= Mat::zeros ( Size(_cols+1, _rows+1), CV_32SC3 );
//
//	Mat _part=_expandedImage(Range(1, _rows), Range(1, _cols));
//	//_part.setTo(a_source);
//	a_source.convertTo(_part, CV_32SC3);
//
//	for( int y = 1; y < _rows; y++ )
//	{
//		for( int x = 1; x < _cols; x++ )
//		{
//			Mat _temp=_expandedImage(Range(y-1, y+2), Range(x-1, x+2));
//			auto _gx=sum(_temp.mul(_dxM));
//			auto _gy=sum(_temp.mul(_dyM));
//			for( int c = 0; c < _channels; c++ )
//			{
//				_retImage.at<Vec3b>(y-1,x-1)[c]=saturate_cast<uchar>( sqrt(_gx[c]*_gx[c]+_gy[c]*_gy[c]));
//			}
//		}
//	}
//	Mat _convertedImage;
//	_retImage.convertTo(_convertedImage, CV_8UC3);
//	return _convertedImage;
//}

//int main( int argc, char** argv )
//{
//	/// Read image given by user
//	//Mat image = imread( argv[1] );
//	Mat image = imread( "touhou-Touhou-Project-anime-Hakurei-Reimu.33p.jpg" );
//	Mat new_image, new_image2;
//
//	/// Do the operation new_image(i,j) = 255 - image(i,j)
//	new_image=negative(image);
//
//	/// Do the operation new_image(i,j) = alpha*image(i,j) + beta
//	new_image2=edgeDetection(image);
//
//	/// Create Windows
//	namedWindow("Original Image", 1);
//	namedWindow("New Image", 1);
//	namedWindow("New Image #2", 1);
//
//	/// Show stuff
//	imshow("Original Image", image);
//	imshow("New Image", new_image);
//	imshow("New Image #2", new_image2);
//	//imwrite("edges.jpg", new_image2);
//
//	/// Wait until user press some key
//	waitKey();
//	return 0;
//}

int main( int argc, char** argv )
{
    VideoCapture cap("D:\\Cool\\Diablo ролики\\Demon_Hunter_RURU.mpg"); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat edges;
    namedWindow("edges",1);
    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
		
        edges=edgeDetection(frame);
        imshow("edges", edges);
        if(waitKey(1) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
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

void negative(Mat a_source)
{
	Vec3b _full(255,255,255);
	int _rows=a_source.rows, _cols=a_source.cols, _channels=a_source.channels();
	for( int y = 0; y < _rows; y++ )
	{
		for( int x = 0; x < _cols; x++ )
		{
			a_source.at<Vec3b>(y,x)=_full-a_source.at<Vec3b>(y,x);
			/*for( int c = 0; c < _channels; c++ )
			{
				a_source.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( 255-a_source.at<Vec3b>(y,x)[c] );
			}*/
		}
	}
}

Mat edgeDetection(Mat a_source)
{
	Mat _gx = (Mat_<double>(3,3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
	Mat _retImage = Mat::zeros( a_source.size(), a_source.type() );
	filter2D(a_source, _retImage, -1, _gx);	
	return _retImage;
}

//class ImageInputFilter: public tbb::filter
//{
//public:
//	ImageInputFilter( VideoCapture* a_capture ) : filter(serial_in_order), capture(a_capture), isVideo(true) {}
//    ~MyInputFilter();
//private:
//    VideoCapture* capture;
//    bool isVideo;
//    /*override*/ void* operator()(void*);
//};
//
//MyInputFilter::MyInputFilter( FILE* input_file_ ) : 
//    filter(serial_in_order),
//    input_file(input_file_),
//    next_slice( TextSlice::allocate( MAX_CHAR_PER_INPUT_SLICE ) )
//{ 
//}
//
//MyInputFilter::~MyInputFilter() {
//    next_slice->free();
//}
// 
//void* MyInputFilter::operator()(void*) {
//    // Read characters into space that is available in the next slice.
//    size_t m = next_slice->avail();
//    size_t n = fread( next_slice->end(), 1, m, input_file );
//    if( !n && next_slice->size()==0 ) {
//        // No more characters to process
//        return NULL;
//    } else {
//        // Have more characters to process.
//        TextSlice& t = *next_slice;
//        next_slice = TextSlice::allocate( MAX_CHAR_PER_INPUT_SLICE );
//        char* p = t.end()+n;
//        if( n==m ) {
//            // Might have read partial number.  If so, transfer characters of partial number to next slice.
//            while( p>t.begin() && isdigit(p[-1]) ) 
//                --p;
//            next_slice->append( p, t.end()+n );
//        }
//        t.set_end(p);
//        return &t;
//    }
//}
//    
////! Filter that changes each decimal number to its square.
//class MyTransformFilter: public tbb::filter {
//public:
//    MyTransformFilter();
//    /*override*/void* operator()( void* item );
//};
//
//MyTransformFilter::MyTransformFilter() : 
//    tbb::filter(parallel) 
//{}  
//
///*override*/void* MyTransformFilter::operator()( void* item ) {
//    TextSlice& input = *static_cast<TextSlice*>(item);
//    // Add terminating null so that strtol works right even if number is at end of the input.
//    *input.end() = '\0';
//    char* p = input.begin();
//    TextSlice& out = *TextSlice::allocate( 2*MAX_CHAR_PER_INPUT_SLICE );
//    char* q = out.begin();
//    for(;;) {
//        while( p<input.end() && !isdigit(*p) ) 
//            *q++ = *p++; 
//        if( p==input.end() ) 
//            break;
//        long x = strtol( p, &p, 10 );
//        // Note: no overflow checking is needed here, as we have twice the 
//        // input string length, but the square of a non-negative integer n 
//        // cannot have more than twice as many digits as n.
//        long y = x*x; 
//        sprintf(q,"%ld",y);
//        q = strchr(q,0);
//    }
//    out.set_end(q);
//    input.free();
//    return &out;
//}
//         
////! Filter that writes each buffer to a file.
//class MyOutputFilter: public tbb::filter {
//    FILE* my_output_file;
//public:
//    MyOutputFilter( FILE* output_file );
//    /*override*/void* operator()( void* item );
//};
//
//MyOutputFilter::MyOutputFilter( FILE* output_file ) : 
//    tbb::filter(serial_in_order),
//    my_output_file(output_file)
//{
//}
//
//void* MyOutputFilter::operator()( void* item ) {
//    TextSlice& out = *static_cast<TextSlice*>(item);
//    size_t n = fwrite( out.begin(), 1, out.size(), my_output_file );
//    if( n!=out.size() ) {
//        fprintf(stderr,"Can't write into file '%s'\n", OutputFileName.c_str());
//        exit(1);
//    }
//    out.free();
//    return NULL;
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
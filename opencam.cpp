#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>


//#include "cvui.h" // opencv header-only gui
#include <ctime>    // fps counter
#include <string>   // std::to_string
#include <map>      // std::map - enum LUT
#include <fstream>  // std::getline
#include <algorithm>
#include "certainty.h"

using namespace cv;
using namespace std;

const string BNN_ROOT_DIR = "/opt/python3.6/lib/python3.6/site-packages/bnn/";
const string BNN_LIB_DIR = "/opt/python3.6/lib/python3.6/site-packages/bnn/src/network/output/sw/";
//const string BNN_LIB_DIR = BNN_ROOT_DIR + "libraries";
const string BNN_BIT_DIR = BNN_ROOT_DIR + "bitstreams";
const string BNN_PARAM_DIR = BNN_ROOT_DIR + "params/cifar10";
const string USER_DIR = "/home/xilinx/BNN/bnn_lib_tests/";

#define WINDOW_WIDTH 176
#define WINDOW_HEIGHT 144
// Bnn expects a 32x32 image
#define bnnSize Size(32, 32)

vector<string> classes;
Mat curFrame;

extern "C" {
	void load_parameters(const char* path);
	vector<unsigned int> inference(const char* path, unsigned int results[64], int number_class, float *usecPerImage);
	float BNN_certainty(const char* path, unsigned int results[64], int number_class, float *usecPerImage);
	unsigned int* inference_multiple(const char* path, int number_class, int *image_number, float *usecPerImage, unsigned int enable_detail);
	void free_results(unsigned int * result);
	void deinit();
}


int streamVideo();
void runBNN(Mat &img, int &output, float &certainty);

void helpMessage(int argc, char** argv)
{
	cout << argv[0] << " <mode> <src>" << endl;
	cout << "mode = image, cam" << endl;
	cout << "if mode is image then src = image source, ignore src for cam" << endl;
}

int main(int argc, char** argv)
{
	for(int i = 0; i < argc; i++)
		cout << "argv[" << i << "]" << " = " << argv[i] << endl;	
    if (argc == 3)
	{
		if (strcmp(argv[1], "image") == 0)
		{
			Mat img = imread(argv[2]);
			int output;
			float certainty;

			runBNN(img, output, certainty);

			return 0;
		}
		else
		{
			helpMessage(argc, argv);
			return -1;
		}
	}
	else if(argc == 2)
	{
		if (strcmp(argv[1], "cam") == 0)
		{
			streamVideo();
		}
		else
		{
			helpMessage(argc, argv);
			return -1;
		}
		
	}
	else
	{
		cout << "argc = " << argc << endl;
		helpMessage(argc, argv);
		return -1;
	}
	
}

int streamVideo()
{
	VideoCapture cap(0);
    // open the default camera, use something different from 0 otherwise;
    if(!cap.open(0))
    {
       cout << "cannot open camera" << endl;
       return 0;
    }
	
	Size frameSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	setNumThreads(1);
	
    for(;;)
    {
		
        Mat curFrame;
        cap >> curFrame;
        if( curFrame.empty() ) break; // end of video stream
		  
		resize(curFrame, curFrame, frameSize); //change to QCIF resolution
		rectangle(curFrame, Point(72, 46), Point(104, 78), Scalar(0, 0, 255)); // draw a 32x32 box at the centre
		
		Rect R(Point(72, 46), Point(104, 78));
		Mat bnnInput(bnnSize, CV_8UC1);
		bnnInput = curFrame(R); //Extract ROI
		int output;
		float certainty;
		
		
		//resize(curFrame(R), bnnInput, Size(32, 32)); //Extract ROI
		// Classify the ROI
		
		runBNN(bnnInput, output, certainty);
		string certainty_s = to_string(certainty);
		//certainty_s.erase(certainty_s.find_last_not_of('0') + 2, string::npos); // Remove trailing zeros
		putText(curFrame, classes[output], Point(55, R.y), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));
		putText(curFrame, certainty_s, Point(55, 90), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255,0));
		
		
        imshow("this is you, smile! :)", curFrame);
		imwrite("ROI.jpg", bnnInput);
		  
        if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
	}
    return 0;
}

float* usecPerImage;
void runBNN(Mat &img, int &output, float &certainty)
{
	
	// Get a list of all the output classes
	ifstream file((BNN_PARAM_DIR + "/classes.txt").c_str());
	cout << "Opening parameters at: " << (BNN_PARAM_DIR + "/classes.txt") << endl;
	string str;
	if (file.is_open())
	{
		
		cout << "Classes: [";
		while (getline(file, str))
		{
			cout << str << ", "; 
			classes.push_back(str);
		}
		cout << "]" << endl;
		
		file.close();
	}
	else
	{
		cout << "Failed to open classes.txt" << endl;
	}
	
	
	vector<unsigned int> BNN_result;
	int certainty_spread = 1;
	
	
	// When opening this file, it will overwrite all previous data.
	ofstream ofs;
	const string filePath = USER_DIR + "tmp.dat";
	cout << filePath << endl;
	ofs.open(filePath);

  	if (ofs.is_open())
  	{
		cout << "Running inference" << std::endl;

		resize(img, img, bnnSize);
		Mat bgr[3];
		split(img, bgr); // Split source (channels)
		// 2:red, 1:green, 0:blue
		ofs << (uint8_t)1;
		ofs << bgr[2];
		ofs << bgr[1];
		ofs << bgr[0];

		BNN_result = inference(filePath.c_str(), (uint8_t)NULL, classes.size(), usecPerImage);
		output = distance(BNN_result.begin(),max_element(BNN_result.begin(), BNN_result.end()));
		
		cout << "Output = " << classes[output] << endl;

		certainty = calculate_certainty(BNN_result, certainty_spread);
		cout << "Certainty = " << certainty << endl;
		
		//cout << *max_element(BNN_result.begin(), BNN_result.end()) << endl;
		
		ofs.close();
	}
	else
	{
		cout << "Error opening tmp file" << endl;
	}
}

#include <opencv.hpp>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <stack>
#define PI 3.1415926

using namespace cv;
using namespace std;

/*-------fourier-mellin transform-----------*/
//************************************
// Method:    phase-correlation match algorithm, calculate offset of two corrletated images
// FullName:  phaseCorr
// Access:    public 
// Returns:   Point, offset of src1&src2
// Qualifier:
// Parameter: const Mat & src1
// Parameter: const Mat & src2
//************************************
Point phaseCorr(const Mat& src1, const Mat& src2);
//************************************
// Method:    2-D fourier transform
// FullName:  fft2
// Access:    public 
// Returns:   cv::Mat
// Qualifier:
// Parameter: const Mat & src
// Parameter: int nonzerorows, speed-up setting, ignore zero rows
//************************************
Mat fft2(const Mat& src, int nonzerorows);
//************************************
// Method:    shift 2-D fourier transform result to image center
// FullName:  shift2center
// Access:    public 
// Returns:   cv::Mat
// Qualifier:
// Parameter: const Mat & src
//************************************
Mat shift2center(const Mat& src);
//************************************
// Method:    create high-pass filter by height&width
// FullName:  highpass_filter
// Access:    public 
// Returns:   cv::Mat
// Qualifier:
// Parameter: int height
// Parameter: int width
//************************************
Mat highpass_filter(int height, int width);
//************************************
// Method:    rotate src to res by angle in degree,
//			  positive values mean counter-clockwise rotation
// FullName:  imrotate
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: const Mat & img
// Parameter: Mat & Res
// Parameter: float angle
//************************************
bool imrotate(const Mat& img, Mat &Res, float angle);
//************************************
// Method:    find maximum value&its location of two images' phrase correlation 
// FullName:  getphaseCorrMaxval_loc
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: InputArray _src1
// Parameter: InputArray _src2
// Parameter: double & maxval, output max-value
// Parameter: Point & maxloc, output maxval-location
//************************************
void getphaseCorrMaxval_loc(InputArray _src1, InputArray _src2, double& maxval, Point& maxloc);
//************************************
// Method:    calculate magnitude of src
// FullName:  magSpectrums
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: InputArray _src
// Parameter: OutputArray _dst
//************************************
void magSpectrums( InputArray _src, OutputArray _dst);
//************************************
// Method:    FF*/|FF*|, F* represents the conjugation of F
// FullName:  divSpectrums
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: InputArray _srcA
// Parameter: InputArray _srcB
// Parameter: OutputArray _dst
// Parameter: int flags
// Parameter: bool conjB
//************************************
void divSpectrums( InputArray _srcA, InputArray _srcB, OutputArray _dst, int flags, bool conjB);
//************************************
// Method:    shift fft result to its center
// FullName:  fftShift
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: InputOutputArray _out
//************************************
void fftShift(InputOutputArray _out);
//************************************
// Method:    Log-Polar Transform, cartesian to log-polar
// FullName:  LogPolarTrans
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: const Mat & src
// Parameter: Mat & dst
// Parameter: Point center
// Parameter: int flags, remap border type
//************************************
void LogPolarTrans(const Mat& src, Mat& dst, Point center, int flags);

//************************************
// Method:    Fourier-Mellin match
// FullName:  FMTmatch
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: const Mat & img1
// Parameter: const Mat & img2
// Parameter: Point2f * offset, output offset
// Parameter: double * theta, output angle in degree
// Parameter: double * scale, useless parameter
//************************************
void FMTmatch(const Mat& img1, const Mat& img2, Point2f* offset, double* theta = 0, double* scale = 0);
void FMTmatchDemo();

/*快速去雾*/
//************************************
// Method:    FastHazeRemoval
// FullName:  FastHazeRemoval
// Access:    public 
// Returns:   cv::Mat
// Qualifier:
// Parameter: const Mat & src, image with haze
// Parameter: float rho1, control parameter
// Parameter: int windowssize, boxfilter windowsize
//************************************
Mat FastHazeRemoval(const Mat& src, float rho1, int windowssize = 101);
//************************************
// Method:    extract min-value in every location of an image 
// FullName:  SpitMinChn
// Access:    public 
// Returns:   cv::Mat, output image
// Qualifier:
// Parameter: const Mat & src, if src's channel is 1, return src, else return the min(RGB)
//************************************
Mat SpitMinChn(const Mat& src);		// 图像预处理，完成单通道到三通道的转换
//************************************
// Method:    Creat Table for look-up, speed-up method
// FullName:  CreatTable
// Access:    public 
// Returns:   cv::Mat
// Qualifier:
// Parameter: float invA
//************************************
Mat CreatTable(float invA);		// Create Mat-Table for look-up
Mat LookUpTable(const Mat& src1, const Mat& src2, const Mat& Table); // Gray image LUT by Mat-Table
Mat LookUpTableC3(const Mat& src1, const Mat& src2, const Mat& Table);	// RGB images LUT by MT
void FastHazeRemovalDemo();

//************************************
// Method:    利用中值滤波去除背景
// FullName:  removeBackground
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: Mat src
// Parameter: Mat & res
// Parameter: int ksize
// Parameter: bool IsAbs
//************************************
void removeBackground(Mat src, Mat& res, int ksize = 15, bool IsAbs = false);

//************************************
// Method:    通过形状角检测图像中的圆
// FullName:  ShapeAngleCircles
// Access:    public 
// Returns:   void
// Qualifier:
// Input: Mat src
// Output: vector<vector<Point>>& circles
// Input: double threshold, shape angle threshold in rad
// Input: int threLength, length of contours threshold
// Input: int thresCanny, up threshold of canny edge detection
//************************************
void ShapeAngleCircles(Mat src, vector<vector<Point>>& circles, double threshold = 0.2, int threLength = 500, int thresCanny = 200);
// 根据圆上点的位置计算圆心和半径，当前使用的是计算所有圆上点的平均值作为圆心，所有点到圆心的距离的平均值作为半径
//************************************
// Method:    Calculate Circle's Parameters
// FullName:  CalCirclePara
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: vector<vector<Point>> circles, circle's location of an image
// Parameter: vector<Point> & centers, output centers of each circle
// Parameter: vector<int> & radius, radius of each circle
//************************************
void CalCirclePara(const vector<vector<Point>> &circles, vector<Point>& centers, vector<int>& radius);
// shape angle circles demo
void ShapeAngleCirclesDemo();

// run-length parameter
typedef struct
{
	long S;      // start position = x*width+y
	long E;      // end position
	long rIndex; // index of the runlength
} Run_length;
//************************************
// Method:    Extract Run-length of a binary image, 0-background&1-foreground
// FullName:  ExtractRunlength
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: InputArray _src, input image
// Parameter: vector<Run_length> & runlength, output run-length
//************************************
void ExtractRunlength(InputArray _src, vector<Run_length> &runlength);
// connected components' properties
typedef struct
{
	long label;                    // label
	long nPixelCnt;                // area
	long xsum, ysum;               // 1nd image moments
	double x0, y0;                 // gravity point
	long xxsum, xysum, yysum;      // 2nd image moments
	double xx, xy, yy;             // fitting ellipse
	int left, top, right, bottom;  // External rectangle
} FEATURES;

//************************************
// Method:    set all parameter to a proper value, not all zeros
// FullName:  InitFeature
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: FEATURES & feature
//************************************
void InitFeature(FEATURES &feature);
//************************************
// Method:    update features by type
// FullName:  Add2Features
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: FEATURES & feature1, input feature
// Parameter: const FEATURES & feature2, feature will be merged
// Parameter: int type, same as StatFeatureInfo
// Parameter: runlength, input runlength that will be added to feature
// Parameter: width, image's width
//************************************
void Add2Features(FEATURES &feature1, const FEATURES &feature2, int type);
void Add2Features(FEATURES &feature1, const Run_length &runlength, int width, int type);
//************************************
// Method:    statistical features of connected components
// FullName:  StatFeatureInfo
// Access:    public 
// Returns:   long
// Qualifier:
// Parameter: InputOutputArray _src
// Parameter: vector<FEATURES> & Features, output features of image's connected components according to type
// Parameter: int type
// Parameter: bool backfill
//************************************
long StatFeatureInfo(InputOutputArray _src, vector<FEATURES> &Features, int type, bool backfill = true);
void StatFeatureInfoDemo();

//************************************
// Method:    find label's Root
// FullName:  findRootIndex
// Access:    public 
// Returns:   long
// Qualifier:
// Parameter: vector<FEATURES> &labels
// Parameter: long position
//************************************
long findRootIndex(vector<FEATURES> &labels, long position);
//************************************
// Method:    set label to the same if pos1&pos2 is connected, 
//            Also update label same as pos1&pos2
// FullName:  unionDCBs
// Access:    public 
// Returns:   long
// Qualifier:
// Parameter: vector<FEATURES> & labels
// Parameter: long pos1
// Parameter: long pos2
//************************************
long unionDCBs(vector<FEATURES> &labels, long pos1, long pos2);
// select method
enum 
{
	REGION_SELECT_AREA = 1, 
	REGION_SELECT_WIDTH = 2, 
	REGION_SELECT_HEIGHT = 4, 
	REGION_SELECT_WIDTH_DIV_HEIGHT = 8
};
// set Feature.label=0 if out of [minThresh, maxThresh]
void RefineFeatureByArea(vector<FEATURES> &Feature, double minThresh, double maxThresh);
void RefineFeatureByWidth(vector<FEATURES> &Feature, double minThresh, double maxThresh);
void RefineFeatureByHeight(vector<FEATURES> &Feature, double minThresh, double maxThresh);
void RefineFeatureByW_DIV_H(vector<FEATURES> &Feature, double minThresh, double maxThresh);

//************************************
// Method:    SelectShape by type and minThresh&maxThresh
// FullName:  SelectShape
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: InputArray _src
// Parameter: OutputArray _dst
// Parameter: vector<Mat> & Regions
// Parameter: int type, same as StatFeatureInfo
// Parameter: const Scalar & minThresh
// Parameter: const Scalar & maxThresh
//************************************
void SelectShape(InputArray _src, OutputArray _dst, vector<Mat> &Regions, int type, const Scalar& minThresh, const Scalar &maxThresh);
//************************************
// Method:    DrawLabelImage, test function
// FullName:  DrawLabelImage
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: const Mat & _labelImg
// Parameter: Mat & _colorLabelImg
//************************************
void DrawLabelImage(const Mat& _labelImg, Mat& _colorLabelImg);
// demo
void SelectShapeDemo();

#include <opencv.hpp>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <stack>
// test
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
void getphaseCorrMaxval_loc(InputArray _src1, InputArray _src2, double& maxval, Point& maxloc);
void magSpectrums( InputArray _src, OutputArray _dst);
void divSpectrums( InputArray _srcA, InputArray _srcB, OutputArray _dst, int flags, bool conjB);
void fftShift(InputOutputArray _out);
// flags is remap border type
void LogPolarTrans(const Mat& src, Mat& dst, Point center, int flags);

void FMTmatch(const Mat& img1, const Mat& img2, Point2f* offset, double* theta = 0, double* scale = 0);
void FMTmatchDemo();

/*快速去雾*/
Mat FastHazeRemoval(const Mat& src, float rho1, int windowssize = 101);
Mat SpitMinChn(const Mat& src);		// 图像预处理，完成单通道到三通道的转换
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
void CalCirclePara(vector<vector<Point>> circles, vector<Point>& centers, vector<int>& radius);
// shape angle circles demo
void ShapeAngleCirclesDemo();

void DrawLabelImage(const Mat& _labelImg, Mat& _colorLabelImg);

/*-----------连通域标号程序未来改写方向--------------
-----------------2016/9/12 reprogram finished----------------
* 1、拆分CCLabeling函数，先提取S、E和Index，为第一个函数
* 2、根据三个结果进行标号和确定需要提取的特征，一般以图像矩表示
* 特征公式参见《快速连通域分析算法及其实现_孔斌》一文
* 3、将minThresh和maxThresh设置成scalar类型或数组型，保存更多参数
*-----------------------------------------------*/
//************************************
// Method:    ExtractRunlength
// FullName:  ExtractRunlength, extract run_length in image
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: uchar * imagedata, first pixel ptr of image 
// Parameter: const long & PixelCount, image's Height * image's Width
// Parameter: long * S, store start number of ith run_length
// Parameter: long * E, store end number of ith run_length
// Parameter: long * rIndex
//************************************
typedef struct
{
	long S;
	long E;
	long rIndex;
} Run_length;
long ExtractRunlength(uchar *imagedata, const long &PixelCount, Run_length *runlength);
void ExtractRunlength(InputArray _src, vector<Run_length> &runlength);
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

void InitFeature(FEATURES &feature);
// update features by type
void Add2Features(FEATURES &feature1, const FEATURES &feature2, int type);
void Add2Features(FEATURES &feature1, const Run_length &runlength, int width, int type);
// statistical features of connected components
long StatFeatureInfo(uchar *image, int Height, int Width, int type, bool backfill = true);
long StatFeatureInfo(InputOutputArray _src, vector<FEATURES> &Features, int type, bool backfill = true);
void StatFeatureInfoDemo();

// 查找根标号的位置
// 更新路径上的元素标号索引
long findRootIndex(FEATURES* labels, long position);
long findRootIndex(vector<FEATURES> &labels, long position);
// 合并两个DCB所在的树
long unionDCBs(FEATURES *labels, long pos1, long pos2);
long unionDCBs(vector<FEATURES> &labels, long pos1, long pos2);


// select regions whose area larger than threshold
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
// the same function with SelectRegion, use the SataFeatureInfo
void SelectShape(InputArray _src, OutputArray _dst, vector<Mat> &Regions, int type, const Scalar& minThresh, const Scalar &maxThresh);
// demo
void SelectShapeDemo();

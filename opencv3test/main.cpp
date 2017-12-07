#include <opencv2/opencv.hpp>
#include <iostream>
#include "Algorithm.h"

using namespace cv;
using namespace std;

int matsize(vector<Mat>& vm)
{
	return vm.size();
}
RNG rng( 12345);
int main()
{
//	ConnComponetLabelDemo();
//	CCLabelingDemo();
//	SelectRegionDemo();
//	FMTmatchDemo();
//	FastHazeRemovalDemo();
//	CylinderExpansionTest();
//	StatFeatureInfoDemo();
//	SelectShapeDemo();
//	floodfilltest();
//	removeBackgroundtest();
//	ShapeAngleCirclesDemo();
	int n = rsaP*rsaQ;
	int e = rsaE;
	int d = cald(rsaP, rsaQ, e);
	cout<<d<<endl;
	vector<int> p;
	int code1[] = {1,23,45,67,89};
	int code2[] = {1,2475,1570,2044,485};
	vector<int> mcode(code1,code1+5);
	vector<int> pp(code2, code2+5);
	// EncryptMachineCode(mcode,n,e,p);
	DecryptMachineCode(pp,n,d,p);
	getchar();
}
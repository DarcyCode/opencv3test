#include <opencv2/opencv.hpp>
#include <iostream>
#include "Algorithm.h"
#include <fstream>
#include "rsadecryption.h"
#include "rsaencryption.h"

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
//	EnDeMcodetest();
// 	arrmcodetest();
//	arrregcodetest();
// 	EnDeMcodetest1();
	int n = rsaP*rsaQ;
	int m = (rsaP-1)*(rsaQ-1);
	int e = rsaE;
	int d = 1873;// ex_gcd(e, m);
	cout << "m:" << m<<"\n";
	cout << "n:" << n<<"\n";
	cout << "d:" << d<<endl;
	string mcode("");
	string regstr("");
	string res("");
	ofstream out;
	out.open("./data.dat", ofstream::out);
	for (long long i = 9999999999; i>1000000000; i--)
	{
		for(long long j = 9999999999; j > 1000000000; j--)
		{
			mcode = to_string(i)+to_string(j);
			regstr = EncryptMachineCode(mcode, n, e);
			res = DecryptMachineCode(regstr,n);
			//out << mcode<<" " << regstr << " " << res;
			if(mcode!=res)
			{
				//out << " false";
				cout << mcode<<" " << regstr << " " << res << endl;
			}
			cout << "1";
			//out << endl;
		}
		cout << "yes" << endl;
	}
	out.close();
	getchar();
}
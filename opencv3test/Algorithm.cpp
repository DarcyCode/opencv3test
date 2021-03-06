#include "Algorithm.h"

Point phaseCorr(const Mat& src1, const Mat& src2)
{
	// size of two images must be the same
	CV_Assert(src1.size() == src2.size());
	Mat padded1,padded2;
	// get the best size of dft
	int m = getOptimalDFTSize(src1.rows);
	int n = getOptimalDFTSize(src1.cols);
	// copy image to padded that have the best dft size
	copyMakeBorder(src1, padded1, 0, m-src1.rows, 0, n-src1.cols, BORDER_CONSTANT, Scalar::all(0));
	copyMakeBorder(src2, padded2, 0, m-src2.rows, 0, n-src2.cols, BORDER_CONSTANT, Scalar::all(0));
	// create 2-channel matrix to store dft complex result
	Mat planes1[] = {Mat_<float>(padded1), Mat::zeros(padded1.size(), CV_32F)};
	Mat planes2[] = {Mat_<float>(padded2), Mat::zeros(padded2.size(), CV_32F)};
	Mat complexI1, complexI2;
	merge(planes1, 2, complexI1);
	merge(planes2, 2, complexI2);
	// excute dft
	dft(complexI1, complexI1);
	dft(complexI2, complexI2);
	// split real part and imag part
	split(complexI1, planes1);
	split(complexI2, planes2);
	// get phase of dft result
	phase(planes1[0], planes1[1], planes1[0]);
	phase(planes2[0], planes2[1], planes2[0]);
	// extract phase-diff
	Mat angle = planes1[0]-planes2[0];
	Mat theta(angle.size(), CV_32FC2, Scalar::all(0));
	// calculate real part&imag part of phase-diff
	int Height = theta.rows;
	int Width = theta.cols;
	if (angle.isContinuous() && theta.isContinuous())
	{
		Width = Width*Height;
		Height = 1;
	}
	float* pfan = angle.ptr<float>(0);
	float* pfth = theta.ptr<float>(0);
	for(int i = 0;i < Height; i++)
	{
		for(int j = 0; j < Width; j++)
		{
			*pfth = cos((*pfan));
			pfth++;
			*pfth = sin((*pfan));
			pfth++;
			pfan++;
		}
	}
	// idft of phase-diff
	dft(theta, theta, DFT_INVERSE+DFT_SCALE);
	// extract max-value's position&return
	split(theta, planes1);
	double minval, maxval;
	Point minloc, maxloc;
	minMaxLoc(planes1[0], &minval, &maxval, &minloc, &maxloc, Mat());
	return maxloc;
}

// 2D fourier transform
cv::Mat fft2(const Mat& src, int nonzerorows)
{
	Mat planes[] = {Mat_<float>(src), Mat::zeros(src.size(), CV_32F)};
	Mat comImg;
	merge(planes, 2, comImg);
	dft(comImg, comImg, 0, nonzerorows);
	return comImg;
}

cv::Mat shift2center(const Mat& src)
{
	// shift to zero frequency
	Mat res = src.clone();
	int cx = res.cols / 2;
	int cy = res.rows / 2;
	Mat q0(res, Rect(0, 0, cx, cy));
	Mat q1(res, Rect(0, cy, cx, cy));
	Mat q2(res, Rect(cx, cy, cx, cy));
	Mat q3(res, Rect(cx, 0, cx, cy));
	Mat tmp;
	q0.copyTo(tmp);
	q2.copyTo(q0);
	tmp.copyTo(q2);

	q1.copyTo(tmp);
	q3.copyTo(q1);
	tmp.copyTo(q3);
	return res;
}

cv::Mat highpass_filter(int height, int width)
{
	// generate high-pass filter
	float resh = 1./(height-1);
	float resw = 1./(width-1);
	Mat eta(height, 1, CV_32F, Scalar::all(0));
	Mat neta(1, width, CV_32F, Scalar::all(0));
	float *pfeta = eta.ptr<float>(0);
	float *pfneta = neta.ptr<float>(0);
	int ch = height/2;
	int cw = width/2;
	for(int i = 0; i < height; i++)
	{
		pfeta[i] = -0.5+i*resh;
		pfeta[i] = cos(CV_PI*pfeta[i]);
	}
	for(int i = 0; i < width; i++)
	{
		pfneta[i] = -0.5+i*resh;
		pfneta[i] = cos(CV_PI*pfneta[i]);
	}
	Mat X(height, width, CV_32F, Scalar::all(0));
	pfeta = eta.ptr<float>(0);
	pfneta = neta.ptr<float>(0);
	float *pfX = X.ptr<float>(0);
	for (int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			*pfX = (pfeta[i])*(pfneta[j]);
			pfX++;
		}
	}
	Mat H(height, width, CV_32F, Scalar::all(0));
	H = (1.0-X).mul(2.0-X);
	return H;
}

bool imrotate(const Mat& img, Mat &Res, float angle)
{
	Point2f center = Point2f(img.cols/2,img.rows/2);
	Mat RotateMat = getRotationMatrix2D(center, angle, 1);
	warpAffine(img,Res,RotateMat,img.size());
	return true;
}

void fftShift(InputOutputArray _out)
{
	Mat out = _out.getMat();
	// out is a number, return
	if(out.rows == 1 && out.cols == 1)
	{
		// trivially shifted.
		return;
	}

	std::vector<Mat> planes;
	split(out, planes);
	// get matrix's center
	int xMid = out.cols >> 1;
	int yMid = out.rows >> 1;

	bool is_1d = xMid == 0 || yMid == 0;
	// if matrix is vector, row or column equals 1
	if(is_1d)
	{
		xMid = xMid + yMid;

		for(size_t i = 0; i < planes.size(); i++)
		{
			Mat tmp;
			Mat half0(planes[i], Rect(0, 0, xMid, 1));
			Mat half1(planes[i], Rect(xMid, 0, xMid, 1));

			half0.copyTo(tmp);
			half1.copyTo(half0);
			tmp.copyTo(half1);
		}
	}
	else
	{
		for(size_t i = 0; i < planes.size(); i++)
		{
			// perform quadrant swaps...
			Mat tmp;
			Mat q0(planes[i], Rect(0,    0,    xMid, yMid));
			Mat q1(planes[i], Rect(xMid, 0,    xMid, yMid));
			Mat q2(planes[i], Rect(0,    yMid, xMid, yMid));
			Mat q3(planes[i], Rect(xMid, yMid, xMid, yMid));

			q0.copyTo(tmp);
			q3.copyTo(q0);
			tmp.copyTo(q3);

			q1.copyTo(tmp);
			q2.copyTo(q1);
			tmp.copyTo(q2);
		}
	}

	merge(planes, out);
}

void magSpectrums( InputArray _src, OutputArray _dst)
{
	Mat src = _src.getMat();
	int depth = src.depth(), cn = src.channels(), type = src.type();
	int rows = src.rows, cols = src.cols;
	int j, k;

	CV_Assert( type == CV_32FC1 || type == CV_32FC2 || type == CV_64FC1 || type == CV_64FC2 );

	if(src.depth() == CV_32F)
		_dst.create( src.rows, src.cols, CV_32FC1 );
	else
		_dst.create( src.rows, src.cols, CV_64FC1 );

	Mat dst = _dst.getMat();
	dst.setTo(0);//Mat elements are not equal to zero by default!

	bool is_1d = (rows == 1 || (cols == 1 && src.isContinuous() && dst.isContinuous()));

	if( is_1d )
		cols = cols + rows - 1, rows = 1;

	int ncols = cols*cn;
	int j0 = cn == 1;
	int j1 = ncols - (cols % 2 == 0 && cn == 1);

	if( depth == CV_32F )
	{
		const float* dataSrc = src.ptr<float>();
		float* dataDst = dst.ptr<float>();

		size_t stepSrc = src.step/sizeof(dataSrc[0]);
		size_t stepDst = dst.step/sizeof(dataDst[0]);

		if( !is_1d && cn == 1 )
		{
			for( k = 0; k < (cols % 2 ? 1 : 2); k++ )
			{
				if( k == 1 )
					dataSrc += cols - 1, dataDst += cols - 1;
				dataDst[0] = dataSrc[0]*dataSrc[0];
				if( rows % 2 == 0 )
					dataDst[(rows-1)*stepDst] = dataSrc[(rows-1)*stepSrc]*dataSrc[(rows-1)*stepSrc];

				for( j = 1; j <= rows - 2; j += 2 )
				{
					dataDst[j*stepDst] = (float)std::sqrt((double)dataSrc[j*stepSrc]*dataSrc[j*stepSrc] +
						(double)dataSrc[(j+1)*stepSrc]*dataSrc[(j+1)*stepSrc]);
				}

				if( k == 1 )
					dataSrc -= cols - 1, dataDst -= cols - 1;
			}
		}

		for( ; rows--; dataSrc += stepSrc, dataDst += stepDst )
		{
			if( is_1d && cn == 1 )
			{
				dataDst[0] = dataSrc[0]*dataSrc[0];
				if( cols % 2 == 0 )
					dataDst[j1] = dataSrc[j1]*dataSrc[j1];
			}

			for( j = j0; j < j1; j += 2 )
			{
				dataDst[j] = (float)std::sqrt((double)dataSrc[j]*dataSrc[j] + (double)dataSrc[j+1]*dataSrc[j+1]);
			}
		}
	}
	else
	{
		const double* dataSrc = src.ptr<double>();
		double* dataDst = dst.ptr<double>();

		size_t stepSrc = src.step/sizeof(dataSrc[0]);
		size_t stepDst = dst.step/sizeof(dataDst[0]);

		if( !is_1d && cn == 1 )
		{
			for( k = 0; k < (cols % 2 ? 1 : 2); k++ )
			{
				if( k == 1 )
					dataSrc += cols - 1, dataDst += cols - 1;
				dataDst[0] = dataSrc[0]*dataSrc[0];
				if( rows % 2 == 0 )
					dataDst[(rows-1)*stepDst] = dataSrc[(rows-1)*stepSrc]*dataSrc[(rows-1)*stepSrc];

				for( j = 1; j <= rows - 2; j += 2 )
				{
					dataDst[j*stepDst] = std::sqrt(dataSrc[j*stepSrc]*dataSrc[j*stepSrc] +
						dataSrc[(j+1)*stepSrc]*dataSrc[(j+1)*stepSrc]);
				}

				if( k == 1 )
					dataSrc -= cols - 1, dataDst -= cols - 1;
			}
		}

		for( ; rows--; dataSrc += stepSrc, dataDst += stepDst )
		{
			if( is_1d && cn == 1 )
			{
				dataDst[0] = dataSrc[0]*dataSrc[0];
				if( cols % 2 == 0 )
					dataDst[j1] = dataSrc[j1]*dataSrc[j1];
			}

			for( j = j0; j < j1; j += 2 )
			{
				dataDst[j] = std::sqrt(dataSrc[j]*dataSrc[j] + dataSrc[j+1]*dataSrc[j+1]);
			}
		}
	}
}

void divSpectrums( InputArray _srcA, InputArray _srcB, OutputArray _dst, int flags, bool conjB)
{
	Mat srcA = _srcA.getMat(), srcB = _srcB.getMat();
	int depth = srcA.depth(), cn = srcA.channels(), type = srcA.type();
	int rows = srcA.rows, cols = srcA.cols;
	int j, k;

	CV_Assert( type == srcB.type() && srcA.size() == srcB.size() );
	CV_Assert( type == CV_32FC1 || type == CV_32FC2 || type == CV_64FC1 || type == CV_64FC2 );

	_dst.create( srcA.rows, srcA.cols, type );
	Mat dst = _dst.getMat();

	bool is_1d = (flags & DFT_ROWS) || (rows == 1 || (cols == 1 &&
		srcA.isContinuous() && srcB.isContinuous() && dst.isContinuous()));

	if( is_1d && !(flags & DFT_ROWS) )
		cols = cols + rows - 1, rows = 1;

	int ncols = cols*cn;
	int j0 = cn == 1;
	int j1 = ncols - (cols % 2 == 0 && cn == 1);

	if( depth == CV_32F )
	{
		const float* dataA = srcA.ptr<float>();
		const float* dataB = srcB.ptr<float>();
		float* dataC = dst.ptr<float>();
		float eps = FLT_EPSILON; // prevent div0 problems

		size_t stepA = srcA.step/sizeof(dataA[0]);
		size_t stepB = srcB.step/sizeof(dataB[0]);
		size_t stepC = dst.step/sizeof(dataC[0]);

		if( !is_1d && cn == 1 )
		{
			for( k = 0; k < (cols % 2 ? 1 : 2); k++ )
			{
				if( k == 1 )
					dataA += cols - 1, dataB += cols - 1, dataC += cols - 1;
				dataC[0] = dataA[0] / (dataB[0] + eps);
				if( rows % 2 == 0 )
					dataC[(rows-1)*stepC] = dataA[(rows-1)*stepA] / (dataB[(rows-1)*stepB] + eps);
				if( !conjB )
					for( j = 1; j <= rows - 2; j += 2 )
					{
						double denom = (double)dataB[j*stepB]*dataB[j*stepB] +
							(double)dataB[(j+1)*stepB]*dataB[(j+1)*stepB] + (double)eps;

						double re = (double)dataA[j*stepA]*dataB[j*stepB] +
							(double)dataA[(j+1)*stepA]*dataB[(j+1)*stepB];

						double im = (double)dataA[(j+1)*stepA]*dataB[j*stepB] -
							(double)dataA[j*stepA]*dataB[(j+1)*stepB];

						dataC[j*stepC] = (float)(re / denom);
						dataC[(j+1)*stepC] = (float)(im / denom);
					}
				else
					for( j = 1; j <= rows - 2; j += 2 )
					{

						double denom = (double)dataB[j*stepB]*dataB[j*stepB] +
							(double)dataB[(j+1)*stepB]*dataB[(j+1)*stepB] + (double)eps;

						double re = (double)dataA[j*stepA]*dataB[j*stepB] -
							(double)dataA[(j+1)*stepA]*dataB[(j+1)*stepB];

						double im = (double)dataA[(j+1)*stepA]*dataB[j*stepB] +
							(double)dataA[j*stepA]*dataB[(j+1)*stepB];

						dataC[j*stepC] = (float)(re / denom);
						dataC[(j+1)*stepC] = (float)(im / denom);
					}
					if( k == 1 )
						dataA -= cols - 1, dataB -= cols - 1, dataC -= cols - 1;
			}
		}

		for( ; rows--; dataA += stepA, dataB += stepB, dataC += stepC )
		{
			if( is_1d && cn == 1 )
			{
				dataC[0] = dataA[0] / (dataB[0] + eps);
				if( cols % 2 == 0 )
					dataC[j1] = dataA[j1] / (dataB[j1] + eps);
			}

			if( !conjB )
				for( j = j0; j < j1; j += 2 )
				{
					double denom = (double)(dataB[j]*dataB[j] + dataB[j+1]*dataB[j+1] + eps);
					double re = (double)(dataA[j]*dataB[j] + dataA[j+1]*dataB[j+1]);
					double im = (double)(dataA[j+1]*dataB[j] - dataA[j]*dataB[j+1]);
					dataC[j] = (float)(re / denom);
					dataC[j+1] = (float)(im / denom);
				}
			else
				for( j = j0; j < j1; j += 2 )
				{
					double denom = (double)(dataB[j]*dataB[j] + dataB[j+1]*dataB[j+1] + eps);
					double re = (double)(dataA[j]*dataB[j] - dataA[j+1]*dataB[j+1]);
					double im = (double)(dataA[j+1]*dataB[j] + dataA[j]*dataB[j+1]);
					dataC[j] = (float)(re / denom);
					dataC[j+1] = (float)(im / denom);
				}
		}
	}
	else
	{
		const double* dataA = srcA.ptr<double>();
		const double* dataB = srcB.ptr<double>();
		double* dataC = dst.ptr<double>();
		double eps = DBL_EPSILON; // prevent div0 problems

		size_t stepA = srcA.step/sizeof(dataA[0]);
		size_t stepB = srcB.step/sizeof(dataB[0]);
		size_t stepC = dst.step/sizeof(dataC[0]);

		if( !is_1d && cn == 1 )
		{
			for( k = 0; k < (cols % 2 ? 1 : 2); k++ )
			{
				if( k == 1 )
					dataA += cols - 1, dataB += cols - 1, dataC += cols - 1;
				dataC[0] = dataA[0] / (dataB[0] + eps);
				if( rows % 2 == 0 )
					dataC[(rows-1)*stepC] = dataA[(rows-1)*stepA] / (dataB[(rows-1)*stepB] + eps);
				if( !conjB )
					for( j = 1; j <= rows - 2; j += 2 )
					{
						double denom = dataB[j*stepB]*dataB[j*stepB] +
							dataB[(j+1)*stepB]*dataB[(j+1)*stepB] + eps;

						double re = dataA[j*stepA]*dataB[j*stepB] +
							dataA[(j+1)*stepA]*dataB[(j+1)*stepB];

						double im = dataA[(j+1)*stepA]*dataB[j*stepB] -
							dataA[j*stepA]*dataB[(j+1)*stepB];

						dataC[j*stepC] = re / denom;
						dataC[(j+1)*stepC] = im / denom;
					}
				else
					for( j = 1; j <= rows - 2; j += 2 )
					{
						double denom = dataB[j*stepB]*dataB[j*stepB] +
							dataB[(j+1)*stepB]*dataB[(j+1)*stepB] + eps;

						double re = dataA[j*stepA]*dataB[j*stepB] -
							dataA[(j+1)*stepA]*dataB[(j+1)*stepB];

						double im = dataA[(j+1)*stepA]*dataB[j*stepB] +
							dataA[j*stepA]*dataB[(j+1)*stepB];

						dataC[j*stepC] = re / denom;
						dataC[(j+1)*stepC] = im / denom;
					}
					if( k == 1 )
						dataA -= cols - 1, dataB -= cols - 1, dataC -= cols - 1;
			}
		}

		for( ; rows--; dataA += stepA, dataB += stepB, dataC += stepC )
		{
			if( is_1d && cn == 1 )
			{
				dataC[0] = dataA[0] / (dataB[0] + eps);
				if( cols % 2 == 0 )
					dataC[j1] = dataA[j1] / (dataB[j1] + eps);
			}

			if( !conjB )
				for( j = j0; j < j1; j += 2 )
				{
					double denom = dataB[j]*dataB[j] + dataB[j+1]*dataB[j+1] + eps;
					double re = dataA[j]*dataB[j] + dataA[j+1]*dataB[j+1];
					double im = dataA[j+1]*dataB[j] - dataA[j]*dataB[j+1];
					dataC[j] = re / denom;
					dataC[j+1] = im / denom;
				}
			else
				for( j = j0; j < j1; j += 2 )
				{
					double denom = dataB[j]*dataB[j] + dataB[j+1]*dataB[j+1] + eps;
					double re = dataA[j]*dataB[j] - dataA[j+1]*dataB[j+1];
					double im = dataA[j+1]*dataB[j] + dataA[j]*dataB[j+1];
					dataC[j] = re / denom;
					dataC[j+1] = im / denom;
				}
		}
	}

}

void getphaseCorrMaxval_loc(InputArray _src1, InputArray _src2, double& maxval, Point& maxloc)
{
	Mat src1 = _src1.getMat();
	Mat src2 = _src2.getMat();

	CV_Assert( src1.type() == src2.type());
	CV_Assert( src1.type() == CV_32FC1 || src1.type() == CV_64FC1 );
	CV_Assert( src1.size == src2.size);

	int M = getOptimalDFTSize(src1.rows);
	int N = getOptimalDFTSize(src1.cols);

	Mat padded1, padded2;

	if(M != src1.rows || N != src1.cols)
	{
		copyMakeBorder(src1, padded1, 0, M - src1.rows, 0, N - src1.cols, BORDER_CONSTANT, Scalar::all(0));
		copyMakeBorder(src2, padded2, 0, M - src2.rows, 0, N - src2.cols, BORDER_CONSTANT, Scalar::all(0));
	}
	else
	{
		padded1 = src1;
		padded2 = src2;
	}

	Mat FFT1, FFT2, P, Pm, C;

	// execute phase correlation equation
	// Reference: http://en.wikipedia.org/wiki/Phase_correlation
	dft(padded1, FFT1, DFT_REAL_OUTPUT);
	dft(padded2, FFT2, DFT_REAL_OUTPUT);

	mulSpectrums(FFT1, FFT2, P, 0, true);

	magSpectrums(P, Pm);
	divSpectrums(P, Pm, C, 0, false); // FF* / |FF*| (phase correlation equation completed here...)

	idft(C, C); // gives us the nice peak shift location...

	fftShift(C); // shift the energy to the center of the frame.

	// locate the highest peak
	Point t;
	minMaxLoc(C, NULL, &maxval, NULL, &t);

	Point center((double)padded1.cols / 2, (double)padded1.rows / 2);

	maxloc = (center - t);
}

void LogPolarTrans(const Mat& src, Mat& dst, Point center, int flags)
{
	CV_Assert(src.type() == CV_32F);
	Size sz = src.size();
	dst.create(sz, CV_32F);
	int i1, i2, i3, i4;
	i1 = sz.width - center.x;
	i2 = center.x - 1;
	i3 = sz.height - center.y;
	i4 = center.y - 1;
	i1 = i1 < i2? i1 : i2;
	i3 = i3 < i4? i3 : i4;
	int d = i1 < i3? i1 : i3;
	Mat rho(sz.height, 1, CV_32F, Scalar::all(0));
	Mat phi(1, sz.width, CV_32F, Scalar::all(0));
	float res_rho = log((double)d)/log(10.)/(sz.height - 1);
	float res_phi = 2.*CV_PI/sz.width;
	int i, j;
	float *pf = rho.ptr<float>(0);
	float ten = 10;
	// rho
	for (i = 1; i < sz.height; i++)
	{
		pf[i] = pf[i-1]+res_rho;
	}
	for (i = 0; i < sz.height; i++)
	{
		pf[i] = pow(ten,pf[i]);
	}
	// phi
	pf = phi.ptr<float>(0);
	for (i = 1; i < sz.width; i++)
	{
		pf[i] = pf[i-1]+res_phi;
	}
	Mat phicos = phi.clone();
	Mat phisin = phi.clone();
	float *pfcos = phicos.ptr<float>(0);
	float *pfsin = phisin.ptr<float>(0);
	for (i = 0; i < sz.width; i++)
	{
		pfcos[i] = cos(pfcos[i]);
		pfsin[i] = sin(pfsin[i]);
	}

	Mat mapx(sz, CV_32F, Scalar::all(0));
	Mat	mapy(sz, CV_32F, Scalar::all(0));
	float *pfrho = rho.ptr<float>(0);
	float *pfphi = phicos.ptr<float>(0);
	float *pfmapx = mapx.ptr<float>(0);
	float *pfmapy = mapy.ptr<float>(0);
	Mat mx;
	for(i = 0; i < sz.height; i++)
	{
		mx = mapx.row(i);
		mx = pfrho[i]*phicos;
// 		for(j = 0; j < sz.width; j++)
// 		{
// 			*pfmapx = pfrho[i] * pfphi[j];
// 			pfmapx++;
// 		}
	}
	add(mapx, center.x-1, mapx);
	pfphi = phisin.ptr<float>(0);
	for(i = 0; i < sz.height; i++)
	{
		mx = mapy.row(i);
		mx = pfrho[i]*phisin;
// 		for(j = 0; j < sz.width; j++)
// 		{
// 			*pfmapy = pfrho[i] * pfphi[j];
// 			pfmapy++;
// 		}
	}
	add(mapy, center.y-1, mapy);

	remap(src, dst, mapx, mapy, flags);
}

void FMTmatch(const Mat& img1, const Mat &img2, Point2f* offs, double* thet /*= 0*/, double* scale /*= 0*/)
{
	CV_Assert(!img1.empty() && !img2.empty() && img1.size() == img2.size());
	int h1 = img1.rows; int w1 = img1.cols;
	int h2 = img2.rows; int w2 = img2.cols;

	// convert to FFT, and shift to zero frequency
	Mat src1,tem1;
	int opHeight = getOptimalDFTSize(h1);
	int opWidth = getOptimalDFTSize(w1);
	if (opHeight == h1&&opWidth == w1)
		src1 = img1.clone();
	else
		copyMakeBorder(img1, src1, 0, opHeight-h1, 0, opWidth-w1, BORDER_CONSTANT, Scalar::all(0));
	copyMakeBorder(img2, tem1, 0, opHeight-h2, 0, opWidth-w2, BORDER_CONSTANT, Scalar::all(0));
	Mat fftsrc = fft2(src1, h1);
	Mat ffttem = fft2(tem1, h2);
	// extract magnitude and shift to center
	Mat magsrc, magtem;
	Mat planes[2];
	split(fftsrc, planes);
	magnitude(planes[0], planes[1], magsrc);
	split(ffttem, planes);
	magnitude(planes[0], planes[1], magtem);
	fftShift(magsrc);
	fftShift(magtem);
	// high-pass filter
	Mat H = highpass_filter(opHeight, opWidth);
	Mat Hsrc = H.mul(magsrc);
	Mat Htem = H.mul(magtem);
	// transform to Log-Polar space;
	Mat Lsrc(Hsrc.size(), CV_32F, Scalar::all(0)), Ltem(Htem.size(), CV_32F, Scalar::all(0));
	LogPolarTrans(Hsrc, Lsrc, Point(opHeight/2, opWidth/2),INTER_NEAREST);
	LogPolarTrans(Htem, Ltem, Point(opHeight/2, opWidth/2),INTER_NEAREST);
	// phase correlation
	Point2d offset = phaseCorrelate(Lsrc, Ltem);
	*scale = floor(offset.y+0.5);
	float theta = (offset.x)*360/Ltem.cols;
	// eliminate 180 ambiguity
	Mat Rimg, Rimg180;
	imrotate(img2, Rimg,theta);
	imrotate(img2, Rimg180, theta+180);

	Rimg.convertTo(Rimg, CV_32F);
	Rimg180.convertTo(Rimg180, CV_32F);
	double maxval,maxval180;
	Point maxloc, maxloc180;
	img1.convertTo(src1, CV_32F);
	getphaseCorrMaxval_loc(Rimg, src1, maxval, maxloc);
	getphaseCorrMaxval_loc(Rimg180, src1, maxval180, maxloc180);
	Mat Rotatedimg;
	if (maxval < maxval180)
	{
		Rotatedimg = Rimg180;
		offset = Point2f(maxloc180);
		if (theta < 180)
			theta = theta + 180;
		else
			theta = theta - 180;
	}
	else
	{
		Rotatedimg = Rimg;
		offset = Point2f(maxloc);
	}

	offs->x = offset.x;
	offs->y = offset.y;
	*thet = theta;
	*scale = pow(10., *scale);
}

void FMTmatchDemo()
{
	/*---------FMT------------*/
	// img1 represents source image, img2 represents template image.
	Mat img1 = imread("D:/images/pic.bmp", 0);
//	removeBackground(img1, img1, 17, false);
	//	imwrite("./2false.bmp", img1);
	// 	imwrite("E:/desktop/samples/001_2.bmp", img1);
	// 	imshow("1", img1);
	// 	waitKey(0);
	// 	return 0;
	//	imrotate(img1,img1, 33);
	Mat img2 = imread("D:/images/pic_cropped_rotated_shifted.bmp", 0);
	if (img1.empty() || img2.empty())
	{
		cout << "Loading image failed" << endl;
		getchar();
		return;
	}
	// 有个奇怪的现象，dft第一次使用时间较长，再次使用时间缩短，而且差别很大
	fft2(Mat(),0);
	double tstart = getTickCount();
	double scale = 0, thet = 0;
	Point2f offs;
	FMTmatch(img1, img2, &offs, &thet, &scale);
	double tend = (getTickCount() - tstart)/getTickFrequency()*1000;
	cout << "offset:" << offs << endl;
	cout << "theta:" <<thet << endl;
	cout << "scale:" <<scale << endl;
	cout << "Time:" << tend << endl;
	Mat Rotatedimg;
	imrotate(img2, Rotatedimg, thet);
	Mat transM = (Mat_<float>(3, 3) << 1, 0, offs.x, 0, 1, offs.y, 0, 0, 1);
	Mat imgsw;
	warpPerspective(Rotatedimg, imgsw, transM, Rotatedimg.size());
	//	imshow("src", img1);
	//	imshow("tem", tem1);
	imgsw.convertTo(imgsw, CV_8U);
	imshow("registered", imgsw/2+img1/2);
	waitKey(0);
//	getchar();
	return;
}

cv::Mat FastHazeRemoval(const Mat& src, float rho1, int windowssize /*= 101*/ )
{
	Mat Img_Src = src.clone();
	Mat Img_Process = SpitMinChn(Img_Src);  // 分离出最小通道，6ms
	// 均值滤波 -- 耗时较长，3ms
	Size sz(windowssize,windowssize);
	Mat AvgMinChn;
	boxFilter(Img_Process, AvgMinChn, CV_32FC1, sz);
	// 求所有元素均值--不需要优化，ms以下
	Scalar Avg = mean(Img_Process)/255;
	// 求
	float rho = rho1*Avg(0);
	float Scale = rho < 0.9?rho:0.9;
	Mat AvgMinChn1 = Scale * AvgMinChn;
	AvgMinChn1.convertTo(AvgMinChn1,CV_8UC1);
	Mat imgL;
	cv::min(Img_Process, AvgMinChn1, imgL);
	double maxSrc, maxAvg,minTmp;
	minMaxLoc(Img_Src,&minTmp,&maxSrc);
	minMaxLoc(AvgMinChn, &minTmp, &maxAvg);
	float invA = 2/(maxSrc + maxAvg);
	Mat Table = CreatTable(invA);
	if (Img_Src.channels() == 1)
	{
		Mat res;
		res = LookUpTable(Img_Src,imgL,Table);
		Img_Process = res.clone();
	}
	else
	{
		Mat img,res;
		img = Img_Src.clone();
		res = LookUpTableC3(img,imgL,Table);
		Img_Process = res.clone();
	}

	return Img_Process;
}

cv::Mat SpitMinChn(const Mat& src )
{
	CV_Assert(!src.empty() && src.depth() == CV_8U && src.isContinuous());
	if (src.channels() == 1)
	{
		return src;  // 单通道直接返回，效果有待测试
	}
	Mat img(src.size(),CV_8UC1);
	/**/
	int end = src.rows * src.cols;
	uchar *uptrsrc = src.data;
	uchar *uptrimg = img.data;
	uchar R,G,B;
	for(int i = 0; i < end; i++)
	{
		{
			B = *uptrsrc++;
			G = *uptrsrc++;
			R = *uptrsrc++;
			*uptrimg++ = R<G?(R<B?R:B):(G<B?G:B);
		}
	}
	return img;
}

cv::Mat CreatTable( float invA )
{
    Mat res(256,256,CV_8UC1);
    uchar *ptr;
    for(int i = 0; i < 256; i++)
    {
		ptr = res.ptr<uchar>(i);
		for(int j = 0; j < 256; j++)
		{
			double val = (i - j) / (1.0 - j*invA);
			val = val > 255?255:val;
			val = val < 0?0:val;
			ptr[j] = static_cast<uchar>(val);
		}
    }
    return res;
}

cv::Mat LookUpTable(const Mat& src1, const Mat& src2, const Mat& Table )
{
	CV_Assert(src1.depth() == CV_8U && src2.depth() == CV_8U && Table.depth() == CV_8U);
	CV_Assert(Table.size() == Size(256,256));
	Mat res(src1.size(),CV_8UC1);
	int Height = src1.rows;
	int Width = src1.cols;
	uchar *ptrres;
	for (int i = 0; i < Height; i++)
	{
		const uchar *ptr1 = src1.ptr<uchar>(i);
		const uchar *ptr2 = src2.ptr<uchar>(i);
		ptrres = res.ptr<uchar>(i);
		for(int j = 0; j < Width; j++)
		{
			const uchar*  ptrtab = Table.ptr<uchar>(ptr1[j]);
			ptrres[j] = ptrtab[ptr2[j]];
		}
	}

	return res;
}

cv::Mat LookUpTableC3(const Mat& src1, const Mat& src2, const Mat& Table )
{
	CV_Assert(src1.depth() == CV_8U && src2.depth() == CV_8U && Table.depth() == CV_8U);
	CV_Assert(Table.size() == Size(256,256));
	Mat res(src1.size(),CV_8UC3);
	int Height = src1.rows;
	int Width = src1.cols;
	uchar *ptr1, *ptr2, *ptrres, *ptrtab, *ptrtmp;
	ptrres = res.data;
	ptr1 = src1.data;
	ptr2 = src2.data;
	ptrtab = Table.data;
	size_t sp = Table.step;
	for (int i = 0; i < Height; i++)
	{
		for(int j = 0; j < Width; j++)
		{
			ptrtmp = ptrtab + (*ptr1++) * sp;
			*ptrres++ = ptrtmp[*ptr2];
			ptrtmp = ptrtab + (*ptr1++) * sp;
			*ptrres++ = ptrtmp[*ptr2];
			ptrtmp = ptrtab + (*ptr1++) * sp;
			*ptrres++ = ptrtmp[*ptr2++];
		}
	}

	return res;
}

void FastHazeRemovalDemo()
{
	fft2(Mat(),0);
	Mat img = imread("D:/images/hazeremove/haze4.jpg");
	Mat imgsw;
	double t = cv::getTickCount();
//	for (int i = 0; i < 50; i++)
	{
		imgsw = FastHazeRemoval(img, 0.9);
	}
	t = cv::getTickCount() - t;
	cout << t*1000/getTickFrequency() << "ms" << endl;
	imshow("1", img);
	imshow("remove", imgsw);
	waitKey(0);
}

void removeBackground(Mat src, Mat& res, int ksize /*= 15*/, bool IsAbs /*= false*/)
{
	CV_Assert(src.depth() == CV_8U);
	res = src.clone();
	medianBlur(res, res, ksize);
	if (IsAbs)
	{
		res = abs(res - src);
		return;
	}
	int nr = src.rows;
	int nc = src.cols;
	if(src.isContinuous())
	{
		nc = nr*nc;
		nr = 1;
	}
	uchar *ptr1 = src.data;
	uchar *ptr2 = res.data;
	for(int i = 0; i < nr; i++)
	{
		for (int j = 0; j < nc; j++)
		{
			*ptr2 = *ptr2 > *ptr1 ? *ptr2 - *ptr1 : 0;
			ptr1++;
			ptr2++;
		}
	}
}

void ShapeAngleCircles(Mat src, vector<vector<Point>>& circles, double threshold /*= 0.2*/, int threLength /*= 500*/, int thresCanny/* = 200*/)
{
	circles.clear();
	Mat image;
	GaussianBlur(src, image,Size(9,9),2,2);
	Mat dx(image.size(),CV_32F);
	Mat dy(image.size(),CV_32F);
	// calculate gradient of image
	Sobel(image,dx,CV_32F,1,0,3);
	Sobel(image,dy,CV_32F,0,1,3);
	// detecte edges by canny
	Mat edges;
	Canny(src, edges, 50,thresCanny);
	imshow("1",edges);
	waitKey(0);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours( edges, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, Point( 0, 0) );
	int i,j;
	for (i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > threLength)
		{
			// calculate gravity of edges
			Moments m;
			m = moments(contours[i]);
			Point2f gravity(m.m10/m.m00, m.m01/m.m00);
			// calculate D_alpha
			float *ptrdx = dx.ptr<float>(0);
			float *ptrdy = dy.ptr<float>(0);
			Point2f pt1, pt2, pt;
			int step = src.step;
			double D_alpha = 0;
			for (j = 0; j < contours[i].size(); j++)
			{
				pt = contours[i][j];
				pt1 = pt - gravity;
				int idx = int(pt.x+pt.y*step);
				pt2 = Point2f(ptrdx[idx], ptrdy[idx]);
				if (norm(pt2)&&norm(pt1))
				{
					double theta = acos(pt1.ddot(pt2)/(norm(pt1)*norm(pt2)));
					D_alpha += theta;
				}
			}
			D_alpha = D_alpha / contours[i].size();
			if (abs(D_alpha)<threshold || abs(D_alpha-CV_PI)<threshold)
			{
				circles.push_back(contours[i]);
			}
		}
	}
}

void CalCirclePara(const vector<vector<Point>> &circles, vector<Point>& centers, vector<int>& radius)
{
	centers.clear();
	radius.clear();
	for (int i = 0; i < circles.size(); i++)
	{
		Point pt(0,0);
		int cnt = circles[i].size();
		for (int j = 0; j < cnt; j++)
			pt += circles[i][j];
		pt = pt/cnt;
		centers.push_back(pt);
	}
	for (int i = 0; i < circles.size(); i++)
	{
		int R = 0;
		Point pt(0,0);
		int cnt = circles[i].size();
		for (int j = 0; j < cnt; j++)
		{
			pt = circles[i][j] - centers[i];
			R += int(norm(pt));
		}
		R = R/cnt;
		radius.push_back(R);
	}
}

void ShapeAngleCirclesDemo()
{
	/************************************************************************/
	/*                   test of detect circles by shape angle              */
	/************************************************************************/
	Mat image = imread("D:/images/samples/pic1.bmp",0);
	vector<vector<Point>> circles;
	int th = 200;
	double t = getTickCount();
	//	while(cin>>th)
	{
		ShapeAngleCircles(image, circles,3,th);
		Mat imgshow(image.size(), CV_8U, Scalar(0));
		cvtColor(imgshow, imgshow, CV_GRAY2BGR);
		for (int i = 0; i < circles.size(); i++) 
		{
			Scalar color( rand() &255, rand() &255, rand() &255 );
			drawContours(imgshow, circles, i, color, 3);
		}
		// 	vector<Point> centers;
		// 	vector<int> radius;
		// 	CalCirclePara(circles, centers, radius);
		// 	for (int i = 0; i < centers.size(); i++)
		// 		cout << "center: " << centers[i] << " radius: " << radius[i] << endl;

		t = (getTickCount()-t)/getTickFrequency()*1000;
		cout << t << "ms" << endl;
		cvtColor(image,image,CV_GRAY2BGR);
		imshow("2", imgshow+image/2);
		waitKey(0);
	}
	return;
}

void DrawLabelImage(const Mat& _labelImg, Mat& _colorLabelImg)
{
	Mat img;
	_labelImg.convertTo(img, CV_8U);
	RNG rng( 12345);
	std::map<uchar, cv::Scalar> colors ;

	int rows = _labelImg.rows ;
	int cols = _labelImg.cols ;

	_colorLabelImg.release() ;
	_colorLabelImg.create(rows, cols, CV_8UC3) ;
	_colorLabelImg = cv::Scalar::all(0);

	for (int i = 0; i < rows; i++)
	{
		const uchar* data_src = (uchar*)img.ptr<uchar>(i) ;
		uchar* data_dst = _colorLabelImg.ptr<uchar>(i) ;
		for (int j = 0; j < cols; j++)
		{
			uchar pixelValue = data_src[j] ;
			if (pixelValue > 0)
			{
				if (colors.count(pixelValue) <= 0)
				{
					colors[pixelValue] = Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255)) ;
				}
				cv::Scalar color = colors[pixelValue] ;
				*data_dst++   = color[0];
				*data_dst++ = color[1];
				*data_dst++ = color[2];
			}
			else
			{
				data_dst++ ;
				data_dst++ ;
				data_dst++ ;
			}
		}
	}
}

void ExtractRunlength(InputArray _src, vector<Run_length> &runlength)
{
	Mat src = _src.getMat();
	long PixelCount = src.rows*src.cols;
	long i, n = 0;
	Run_length rr;
	uchar *imagedata = src.data;
	for(i=0; i<PixelCount; i++)
	{
		if(imagedata[i])
		{
			rr.S = i;
			i++;
			while(imagedata[i])
				i++;
			rr.E = i-1;
			rr.rIndex = n;
			runlength.push_back(rr);
			n++;
		}
	}
}


void InitFeature(FEATURES &feature)
{
	feature.label = 0;
	feature.bottom = 0;
	feature.left = 9999;
	feature.nPixelCnt = 0;
	feature.right = 0;
	feature.top = 9999;
	feature.x0 = 0;
	feature.xsum = 0;
	feature.xx = 0;
	feature.xxsum = 0;
	feature.xy = 0;
	feature.xysum = 0;
	feature.y0 = 0;
	feature.ysum = 0;
	feature.yy = 0;
	feature.yysum = 0;
}

void Add2Features(FEATURES &feature1, const FEATURES &feature2, int type)
{
	switch(type)
	{
	case REGION_SELECT_AREA:
		if (feature1.label != feature2.label)
			feature1.nPixelCnt += feature2.nPixelCnt;
		break;
	// compare top&bottom of feature1&feature2
	case REGION_SELECT_HEIGHT:
		feature1.top=min(feature1.top, feature2.top);
		feature1.bottom=max(feature1.bottom, feature2.bottom);
		break;
	case REGION_SELECT_WIDTH:
		feature1.left=min(feature1.left, feature2.left);
		feature1.right=max(feature1.right, feature2.right);
		break;
	case REGION_SELECT_WIDTH_DIV_HEIGHT:
		feature1.top=min(feature1.top, feature2.top);
		feature1.bottom=max(feature1.bottom, feature2.bottom);
		feature1.left=min(feature1.left, feature2.left);
		feature1.right=max(feature1.right, feature2.right);
		break;
	default:
		break;
	}
}

void Add2Features(FEATURES &feature1, const Run_length &runlength, int width, int type)
{
	long a = 0; // temporary variable
	switch(type)
	{
	case REGION_SELECT_AREA:
		feature1.nPixelCnt += runlength.E-runlength.S+1;
		break;
	case REGION_SELECT_HEIGHT:
		feature1.top=min(feature1.top, int(runlength.S/width));
		feature1.bottom=max(feature1.bottom, int(runlength.S/width));
		break;
	case REGION_SELECT_WIDTH:
		feature1.left=min(feature1.left, int(runlength.S%width));
		feature1.right=max(feature1.right, int(runlength.E%width));
		break;
	case REGION_SELECT_WIDTH_DIV_HEIGHT:
		feature1.top=min(feature1.top, int(runlength.S/width));
		feature1.bottom=max(feature1.bottom, int(runlength.S/width));
		feature1.left=min(feature1.left, int(runlength.S%width));
		feature1.right=max(feature1.right, int(runlength.E%width));
		break;
	default:
		break;
	}
}

long StatFeatureInfo(InputOutputArray _src, vector<FEATURES> &Features, int type, bool backfill/* = true*/)
{
	Mat src = _src.getMat();
	long M = src.rows, N = src.cols;

	long runSize = M/2 * N+1;
	
	long t, runCnt;
	vector<Run_length> runlength; 
	ExtractRunlength(src, runlength);
	runCnt = runlength.size();
	FEATURES feature;
	InitFeature(feature);
	Features.push_back(feature);

	long  n=0, l=1, j=0;                       //l:标号从1开始，以免0与背景色0重复
	//
	for(n=0; n<runCnt; n++)
	{

		while(runlength[j].E < runlength[n].S-N-1)   //第一个v>=s-N的j
			j++;

		if (runlength[j].E <= runlength[n].E-N)//if(E[j] <= E[n]-N)       //rj在rn之前结束
		{
			Add2Features(Features[runlength[j].rIndex], runlength[n], N, type);
			runlength[n].rIndex = runlength[j].rIndex;
			t=j;
			j++;
			while(runlength[j].E <= runlength[n].E-N)// while(E[j] <= E[n]-N)
			{
				Add2Features(Features[runlength[t].rIndex], Features[runlength[j].rIndex], type);
				unionDCBs(Features, runlength[t].rIndex, runlength[j].rIndex);
				j++;
			}

			if(runlength[j].S <= runlength[n].E-N+1)//if(S[j] <= E[n]-N+1)
			{
				Add2Features(Features[runlength[t].rIndex], Features[runlength[j].rIndex], type);
				unionDCBs(Features, runlength[t].rIndex, runlength[j].rIndex);
			}
		}
		else
		{
			if(runlength[j].S <= runlength[n].E-N+1)//if(S[j] <= E[n]-N+1)    //u<=e-N?
			{
				Add2Features(Features[runlength[j].rIndex], runlength[n], N, type);
				runlength[n].rIndex = runlength[j].rIndex;
			}
			else
			{
				runlength[n].rIndex = l;
				InitFeature(feature);
				feature.label = l;
				// Features[l].nPixelCnt = 0;
				Add2Features(feature, runlength[n], N, type);
				Features.push_back(feature);
				l++;
			}
		}
	}

	long cComponentCount = 0;
	long k, c0, x;
	for(k=1; k<l; ++k)             //更新所有等价分支的标号
	{
		if(Features[k].label == k)
			++cComponentCount;
		else
			Features[k].label = Features[findRootIndex(Features, Features[k].label)].label;
	}
	// 不回填时，label存放的就是各连通分支的标号
	// 回填到图像，使每个像素得到其标号值
	if(backfill)
	{
		uchar *image = src.data;
		for(k=0; k<runCnt; k++)
		{
			c0 = Features[runlength[k].rIndex].label;
			for(x=runlength[k].S; x<=runlength[k].E; x++)
				image[x] = c0;
		}
	}
	return cComponentCount;   //返回连通域个数
}

void StatFeatureInfoDemo()
{
	Mat img = imread("D:/images/test1.tif", 0);
	Mat bw;
	threshold(img, bw, 100, 1, THRESH_OTSU+THRESH_BINARY);
	double t = cv::getTickCount();
	Mat img1(img.rows, img.cols+1, CV_8U, Scalar(0));
	copyMakeBorder(bw,img1,0,0,0,1, BORDER_CONSTANT, Scalar(0));
	Mat img2 = img1.clone();
	uchar *pimg = img1.ptr<uchar>(0);
	long cnt;
//	cnt = StatFeatureInfo(pimg, img1.rows, img1.cols, REGION_SELECT_WIDTH, true);
	vector<FEATURES> Feature;
	cnt = StatFeatureInfo(img1, Feature, REGION_SELECT_HEIGHT, true);
	t = cv::getTickCount() - t;
	cout << t*1000/getTickFrequency() << "ms" << endl;
	cout << cnt << endl;
	Mat imgsw;
	DrawLabelImage(img1, imgsw);
	namedWindow("1", 0);
	imshow("1", imgsw);
	waitKey(0);
}

long findRootIndex(vector<FEATURES> &labels, long position)
{
	//递归版本
	if(labels[position].label != position)
	{
		labels[position].label = findRootIndex(labels, labels[position].label);
		return labels[position].label;
	}
	return position;
}

long unionDCBs(vector<FEATURES> &labels, long pos1, long pos2)
{
	long x = findRootIndex(labels, pos1);
	long y = findRootIndex(labels, pos2);

	if(x == y)
		return  x;
	// labels will be unioned, so ensuring the same connected 
	// component has only one label is enough
	labels[y].label = x;
	return  x;
}

void SelectShape(InputArray _src, OutputArray _dst, vector<Mat> &Regions, int type, const Scalar& minThresh, const Scalar &maxThresh)
{
	Mat src = _src.getMat();
	// add a column in the right of image
	_dst.create(src.rows, src.cols+1, CV_8U);
	Mat dst = _dst.getMat();
	copyMakeBorder(src, dst, 0, 0, 0, 1, BORDER_CONSTANT, 0);
	vector<FEATURES> Feature;
	StatFeatureInfo(dst, Feature, type, true);
	switch(type)
	{
	case REGION_SELECT_AREA:
		RefineFeatureByArea(Feature, minThresh[0], maxThresh[0]);
		break;
	case REGION_SELECT_WIDTH:
		RefineFeatureByWidth(Feature, minThresh[0], maxThresh[0]);
		break;
	case REGION_SELECT_HEIGHT:
		RefineFeatureByHeight(Feature, minThresh[0], maxThresh[0]);
		break;
	case REGION_SELECT_WIDTH_DIV_HEIGHT:
		RefineFeatureByW_DIV_H(Feature, minThresh[0], maxThresh[0]);
		break;
	default:
		break;
	}
	// pop the right column of dst
	dst = dst.t();
	dst.pop_back();
	dst = dst.t();
	for (int i = 1; i < Feature.size(); i++)
	{
		if (Feature[i].label == i)
		{
			Regions.push_back(dst == i);
		}
	}
	_dst.create(dst.size(),CV_8U);
	dst = _dst.getMat();
	dst.setTo(0);
	for (int i = 0; i < Regions.size(); i++)
	{
		dst += Regions[i];
	}
}

void RefineFeatureByArea(vector<FEATURES> &Feature, double minThresh, double maxThresh)
{
	for (int i = 0; i < Feature.size(); i++)
	{
		if (Feature[i].label==i)
		{
			long Area = Feature[i].nPixelCnt;
			if (Area<minThresh||Area>maxThresh)
				Feature[i].label = 0;
		}
	}
}

void RefineFeatureByWidth(vector<FEATURES> &Feature, double minThresh, double maxThresh)
{
	for (int i = 0; i < Feature.size(); i++)
	{
		if (Feature[i].label==i)
		{
			int Width = Feature[i].right-Feature[i].left+1;
			if (Width<minThresh||Width>maxThresh)
				Feature[i].label = 0;
		}
	}
}

void RefineFeatureByHeight(vector<FEATURES> &Feature, double minThresh, double maxThresh)
{
	for (int i = 0; i < Feature.size(); i++)
	{
		if (Feature[i].label==i)
		{
			int Height = Feature[i].bottom-Feature[i].top+1;
			if (Height<minThresh||Height>maxThresh)
				Feature[i].label = 0;
		}
	}
}

void RefineFeatureByW_DIV_H(vector<FEATURES> &Feature, double minThresh, double maxThresh)
{
	for (int i = 0; i < Feature.size(); i++)
	{
		if (Feature[i].label==i)
		{
			int Width = Feature[i].right-Feature[i].left+1; 
			int Height = Feature[i].bottom-Feature[i].top+1;
			double rate = Width*1./Height;
			if (rate<minThresh||rate>maxThresh)
				Feature[i].label = 0;
		}
	}
}

void SelectShapeDemo()
{
	Mat img = imread("D:/images/test1.tif",0);
	Mat bw;
	threshold(img, bw, 100, 1, THRESH_OTSU+THRESH_BINARY);
	double t = cv::getTickCount();
	vector<Mat> regions;
//	for (int i = 0; i < 1000; i++)
	{
		regions.clear();
		SelectShape(bw, img, regions, REGION_SELECT_WIDTH_DIV_HEIGHT, 0,2);
	}
	t = cv::getTickCount() - t;
	cout << t*1000/getTickFrequency() << "ms" << endl;
	imshow("before", bw*255);
	imshow("after", (img));
	waitKey(0);
}

void CylinderExpansion(InputArray _src, OutputArray _dst, int R, int a, int b, float Theta)
{
	
	// verify
	Mat src = _src.getMat();
	CV_Assert(src.depth() == CV_8UC1);
	CV_Assert((a+b)>0);
	// some important vars
	//	R = src.cols/2;
	R = src.cols/(2.*sin(Theta/2));
	int Width = Theta*R;
	// create dst
	_dst.create(src.rows, Width, CV_8U);
	Mat dst = _dst.getMat();
	// establish mapping
	float theta = 0.;
	Mat mapx(1, Width, CV_32FC1, Scalar(0));
	float *ptr = mapx.ptr<float>(0);
	for (int i = 1; i <= Width/2; i++)
	{
		theta = 1.*(i+1)/R+(CV_PI-Theta)/2;
		ptr[i] = R*(sin(Theta/2)-cos(theta));
		ptr[Width-1-i] = src.cols-ptr[i];
	}
	ptr[Width/2+1] = src.cols/2+1;
	Mat map_y(dst.size(), CV_32FC1);
	int Height = map_y.rows;
	Width = map_y.cols;
	ptr = map_y.ptr<float>(0);
	int Middle;
	if (a*b<0)
	{
		for(int i = 0; i < Height; i++)
		{
			int tmp = a<0?-a:-b;
			int sgn = a<0?-1:1;
			float delta = 1.*(i+1)*(b+a)/Height+tmp;
			for(int j = 0; j <= Width/2; j++)
			{
				ptr[j] = i+sgn*(1-2.*(j+1)/Width)*delta;
				ptr[Width-j-1] = ptr[j];
			}
			ptr[Width/2+1]=0;
			ptr += Width;
		}
	}
	else
	{
		Middle = 1.*a*Height/(a+b);
		for(int i = 0; i < Middle; i++)
		{
			float delta = (1-1.*(i+1)/Middle)*a;
			for(int j = 0; j <= Width/2; j++)
			{
				ptr[j] = i+(1-2.*(j+1)/Width)*delta;
				ptr[Width-j-1] = ptr[j];
			}
			ptr[Width/2+1]=0;
			ptr += Width;
		}
		for(int i = Middle; i < Height; i++)
		{
			float delta = 1.*(i+1-Middle)/(Height-Middle)*b;
			for(int j = 0; j <= Width/2; j++)
			{
				ptr[j] = i-(1-2.*(j+1)/Width)*delta;
				ptr[Width-j-1] = ptr[j];
			}
			ptr[Width/2+1]=0;
			ptr += Width;
		}
	}
	uchar *ptrsrc = src.data;
	uchar *ptrdst = dst.data;
	float *ptrmapx, *ptrmapy;
	ptrmapx = mapx.ptr<float>(0);
	int w = src.cols;

	for(int i = 0; i <= Width/2; i++)
	{
		ptrdst = dst.ptr<uchar>(0);
		ptrmapy = map_y.ptr<float>(0);
		int x1 = static_cast<int>(ptrmapx[i]);
		float w1 = ptrmapx[i]-x1;
		for(int j = 0; j < Height; j++)
		{
			int y1;
			float w2;
			y1 = static_cast<int>(ptrmapy[i]);
			if (y1<0||y1>Height)
			{
				ptrdst[i]=0;
				ptrdst[Width-i-1]=0;
			}
			else
			{
				w2 = ptrmapy[i]-y1;
				int baseloc = x1+y1*w;
				ptrdst[i] = static_cast<uchar>(w1*w2*ptrsrc[baseloc+1+w]+(1.-w1)*w2*ptrsrc[baseloc+w]+w1*(1.-w2)*ptrsrc[baseloc+1]+(1.-w1)*(1.-w2)*ptrsrc[baseloc]);
				baseloc = w-1-x1+y1*w;
				ptrdst[Width-i-1] = static_cast<uchar>(w1*w2*ptrsrc[baseloc+w-1]+(1.-w1)*w2*ptrsrc[baseloc+w]+w1*(1.-w2)*ptrsrc[baseloc-1]+(1.-w1)*(1.-w2)*ptrsrc[baseloc]);
			}
			//if (j < Height-1)
			{
 				ptrdst += Width;
				ptrmapy += Width;
			}
		}
	}
	src.col(src.cols/2+1).copyTo(dst.col(Width/2+1));
}

void CylinderExpansionTest()
{
	Mat src = imread("D:/images/cylinderexpansion/biaoqian2.bmp", 0);
	Mat dst;
	double t = getTickCount();
	for(int i = 0; i < 1000; i++)
		CylinderExpansion(src, dst, src.cols/2,-10,30,2.8/3*CV_PI);
	t = (getTickCount()-t)/getTickFrequency()*1;
	cout << t << "ms" << endl;
	imshow("1", src);
	imshow("dst", dst);
	waitKey(0);
/*	Mat src(1,250,CV_8U,Scalar(0));
	uchar *ptr = src.data;
	for (int i = 0; i < src.cols; i++)
	{
		ptr[i] = i;
	}
	cout << src << endl;
	Mat dst;
	double t = getTickCount();
	CylinderExpansionNremap(src, dst, src.cols/2);
	t = (getTickCount()-t)/getTickFrequency()*1000;
	cout << dst << endl;
	cout << t << "ms" << endl;
*/
}
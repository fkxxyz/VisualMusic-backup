
// DomainTransform.h
#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <cassert>

// 样本数必须是 2 的幂，r 为指数
class CFFT {

private:
	int R, N;

public:
	CFFT(int r);
	~CFFT();

	static int POWER_2[32];
	static std::complex<double> *all_buf[32];
	static int n_Object;

	template <class TYPE>
	void FFT(TYPE *TD, std::complex<double> *FD);
	template <class TYPE, class FTYPE>
	void FFT(TYPE *TD, FTYPE *FD);
	void IFFT(std::complex<double> *FD, std::complex<double> *TD);
};

class CFFT_2D {

private:
	CFFT fft1;
	CFFT fft2;

	int R1, R2;

public:
	CFFT_2D(int r1, int r2):R1(r1),R2(r2),fft1(r1),fft2(r2){}

	void FFT_2D(std::complex<double> *TD, std::complex<double> *FD);
	void IFFT_2D(std::complex<double> *FD, std::complex<double> *TD);
};


void DFT(std::complex<double> * TD, std::complex<double> * FD, int r);

void DCT(double *f, double *F, int r);

void HWT(double *f, double *F, int r);
void IHWT(double *F, double *f, int r);

void HWT_2D(double *f, double *F, int r1, int r2);

//void WALSH(double *f, double *F, int r, double *pBuffer);
//void IWALSH(double *F, double *f, int r);

//void WALSH_2D(double *f, double *F, int r1, int r2, double *pBuffer);


#include "DomainTransform.inl"


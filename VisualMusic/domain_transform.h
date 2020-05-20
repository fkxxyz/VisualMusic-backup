
// domain_transform.h
#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <cassert>

// 样本数必须是 2 的幂，r 为指数
class CFFT {
 public:
  CFFT(const int r);
  ~CFFT();

  static int POWER_2[32];
  static std::complex<double> *all_buf[32];
  static int n_Object;

  template <class TYPE>
  void FFT(const TYPE *TD, std::complex<double> *FD);
  template <class TYPE, class FTYPE>
  void FFT(const TYPE *TD, FTYPE *FD);
  void IFFT(const std::complex<double> *FD, std::complex<double> *TD);

 private:
  int R, N;

};

class CFFT_2D {
 public:
  CFFT_2D(const int r1, const int r2):R1(r1),R2(r2),fft1(r1),fft2(r2) {}

  void FFT_2D(std::complex<double> *TD, std::complex<double> *FD);
  void IFFT_2D(std::complex<double> *FD, std::complex<double> *TD);

 private:
  CFFT fft1;
  CFFT fft2;

  int R1, R2;

};

void DFT(const std::complex<double> * TD, std::complex<double> * FD,
         const int r);

void DCT(const double *f, double *F, const int r);

// 带步进的数据类型复制
template <typename type>
inline void dscopy(type *dest, const type *src,
                   const int n, const int step = 1) {
  register type *p_dest_end = dest + n * step;
  while (dest < p_dest_end) {
    *dest = *src;

    dest += step;
    src += step;
  }
}




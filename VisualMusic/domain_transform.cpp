
// DomainTransform.cpp

#include "stdafx.h"
#include "domain_transform.h"

int CFFT::POWER_2[32] = {
  0x00000001, 0x00000002, 0x00000004, 0x00000008,
  0x00000010, 0x00000020, 0x00000040, 0x00000080,
  0x00000100, 0x00000200, 0x00000400, 0x00000800,
  0x00001000, 0x00002000, 0x00004000, 0x00008000,
  0x00010000, 0x00020000, 0x00040000, 0x00080000,
  0x00100000, 0x00200000, 0x00400000, 0x00800000,
  0x01000000, 0x02000000, 0x04000000, 0x08000000,
  0x10000000, 0x20000000, 0x40000000, 0x80000000
};

std::complex<double> *CFFT::all_buf[32] = {0};
int CFFT::n_Object = 0;

void DFT(const std::complex<double> *TD,
         std::complex<double> *FD, const int N) {
  int k, n;

  // 分配运算所需存储器
  std::complex<double> *W  = new std::complex<double>[N * N];

  // 计算加权系数
  for(k = 0; k < N; k++) {
    for(n = 0; n < N; n++) {
      int i = k * n;
      double angle = - M_PI * 2 * i / N;
      W[i] = std::complex<double>(cos(angle), sin(angle));
    }
  }

  // 采用DFT公式进行变换
  for (k = 0; k < N; k++) {
    std::complex<double> fdi(0, 0);
    for (n = 0; n < N; n++) {
      fdi += TD[n] * W[k * n];
    }
    FD[k] = fdi;
  }

  // 释放内存
  delete[] W;
}

void IDFT(const std::complex<double> *FD,
          std::complex<double> *TD, const int N) {
  int k, n;

  // 分配运算所需存储器
  std::complex<double> *IW  = new std::complex<double>[N * N];

  // 计算加权系数
  for(k = 0; k < N; k++) {
    for(n = 0; n < N; n++) {
      int i = k * n;
      double angle = M_PI * 2 * i / N;
      IW[i] = std::complex<double>(cos(angle), sin(angle));
    }
  }

  // 采用DFT公式进行变换
  for (k = 0; k < N; k++) {
    std::complex<double> fdi(0, 0);
    for (n = 0; n < N; n++) {
      fdi += FD[n] * IW[k * n];
    }
    TD[k] = fdi / (double)N;
  }

  // 释放内存
  delete[] IW;
}


CFFT::CFFT(const int r):
  R(r),
  N(1<<R)
{
  assert(r >= 0 && r < 32);

  if (!all_buf[r]) {
    register std::complex<double> *W =
      all_buf[r] = new std::complex<double>[N * 3];

    // 计算加权系数
    for(register int i = 0; i < N; i++) {
      register double angle = M_PI * 2 * i / N;
      W[i] = std::complex<double>(cos(angle), -sin(angle));
    }
  }

  n_Object++;
}


CFFT::~CFFT() {
  n_Object--;

  if (n_Object == 0)
    for (int i = 0; i < 32; i++)
      if (all_buf[i]) {
        delete all_buf[i];
        all_buf[i] = NULL;
      }
}


template <class TYPE>
void CFFT::FFT(const TYPE *TD, std::complex<double> *FD) {

  // 设置中间变量
  register std::complex<double> *X1 = all_buf[R] + N;
  register std::complex<double> *X2 = X1 + N;

  // 将时域点写入X1
  const TYPE *src_end = TD + N;
  const TYPE *src = TD;
  register std::complex<double> *dst = X1;
  while (src < src_end)
    *(dst++) = *(src++);

  // 蝶形算法
  register std::complex<double> *W = all_buf[R];
  for(register int k = 0; k < R; k++) {
    register int bfsize = POWER_2[R - k - 1];
    for(register int j = 0; j < POWER_2[k]; j++) {
      register int p = j * POWER_2[R - k];
      for(register int i = 0; i < bfsize; i++) {
        X2[i + p] = X1[i + p] + X1[i + p + bfsize];
        X2[i + p + bfsize] =
            (X1[i + p] - X1[i + p + bfsize]) * W[i * POWER_2[k]];
      }
    }

    // 交换 X1 与 X2
    register std::complex<double> *X  = X1;
    X1 = X2;
    X2 = X;
  }

  // 重新排序
  for(register int j = 0; j < N; j++) {
    register int p = 0;
    for(register int i = 0; i < R; i++)
      if (j & POWER_2[i])
        p += POWER_2[R - i - 1];

    FD[j] = X1[p];
  }
}

template <class TYPE, class FTYPE>
void CFFT::FFT(const TYPE *TD, FTYPE *FD) {

  // 设置中间变量
  register std::complex<double> *X1 = all_buf[R] + N;
  register std::complex<double> *X2 = X1 + N;

  // 将时域点写入X1
  register TYPE *src_end = TD + N;
  register TYPE *src = TD;
  register std::complex<double> *dst = X1;
  while (src < src_end)
    *(dst++) = *(src++);

  // 蝶形算法
  register std::complex<double> *W = all_buf[R];
  for(register int k = 0; k < R; k++) {
    register int bfsize = POWER_2[R - k - 1];
    for(register int j = 0; j < POWER_2[k]; j++) {
      register int p = j * POWER_2[R - k];
      for(register int i = 0; i < bfsize; i++) {
        X2[i + p] = X1[i + p] + X1[i + p + bfsize];
        X2[i + p + bfsize] = 
            (X1[i + p] - X1[i + p + bfsize]) * W[i * POWER_2[k]];
      }
    }

    // 交换 X1 与 X2
    register std::complex<double> *X  = X1;
    X1 = X2;
    X2 = X;
  }

  // 重新排序
  for(register int j = 0; j < N; j++) {
    register int p = 0;
    for(register int i = 0; i < R; i++)
      if (j & POWER_2[i])
        p += POWER_2[R - i - 1];

    FD[j] = abs(X1[p]);
  }
}


void CFFT::IFFT(const std::complex<double> *FD,
                std::complex<double> *TD) {

  // 设置中间变量
  register std::complex<double> *X1 = all_buf[R] + N;
  register std::complex<double> *X2 = X1 + N;

  // 将频域点写入X1
  register const std::complex<double> *src_end = FD + N;
  register const std::complex<double> *src = FD;
  register std::complex<double> *dst = X1;
  while (src < src_end)
    *(dst++) = *(src++);

  // 蝶形算法
  register std::complex<double> *W = all_buf[R];
  for(register int k = 0; k < R; k++) {
    register int bfsize = POWER_2[R - k - 1];
    for(register int j = 0; j < POWER_2[k]; j++) {
      register int p = j * POWER_2[R - k];
      for(register int i = 0; i < bfsize; i++) {
        X2[i + p] = X1[i + p] + X1[i + p + bfsize];
        X2[i + p + bfsize] =
            (X1[i + p] - X1[i + p + bfsize]) * conj(W[i * POWER_2[k]]);
      }
    }

    // 交换 X1 与 X2
    register std::complex<double> *X  = X1;
    X1 = X2;
    X2 = X;
  }

  // 重新排序
  for(register int j = 0; j < N; j++) {
    register int p = 0;
    for(register int i = 0; i < R; i++)
      if (j & POWER_2[i])
        p += POWER_2[R - i - 1];

    TD[j] = X1[p] / (double)N;
  }
}


void CFFT_2D::FFT_2D(std::complex<double> *TD,
                     std::complex<double> *FD) {
  int i, j;

  int N1 = 1 << R1;
  int N2 = 1 << R2;

  // 对第一维度进行傅立叶变换
  for (j = 0; j < N2; j++)
    fft1.FFT(&TD[j * N1], &FD[j * N1]);

  // 翻转以便进行第二维变换
  for (i = 0; i < N1; i++)
    for (j = 0; j < N2; j++)
      TD[j + i * N2] = FD[i + j * N1];

  // 对第二维度进行傅立叶变换
  for (i = 0; i < N1; i++)
    fft2.FFT(&TD[i * N2], &FD[i * N2]);

  // 再次翻转得到最终二维变换结果
  memcpy(TD, FD, N1 * N2 * sizeof(std::complex<double>));
  for (j = 0; j < N2; j++)
    for (i = 0; i < N1; i++)
      FD[i + j * N1] = TD[j + i * N2];
}

// 二维傅立叶反变换 （需要提供 3 * 2^max(r1,r2) * sizeof(std::complex<double>) 大小的缓冲区）
void CFFT_2D::IFFT_2D(std::complex<double> *FD, std::complex<double> *TD) {
  int i, j;

  int N1 = 1 << R1;
  int N2 = 1 << R2;

  // 对第一维度进行傅立叶反变换
  for (j = 0; j < N2; j++)
    fft1.IFFT(&FD[j * N1], &TD[j * N1]);

  // 翻转以便进行第二维变换
  for (i = 0; i < N1; i++)
    for (j = 0; j < N2; j++)
      FD[j + i * N2] = TD[i + j * N1];

  // 对第二维度进行傅立叶反变换
  for (i = 0; i < N1; i++)
    fft2.IFFT(&FD[i * N2], &TD[i * N2]);

  // 再次翻转得到最终二维变换结果
  memcpy(FD, TD, N1 * N2 * sizeof(std::complex<double>));
  for (j = 0; j < N2; j++)
    for (i = 0; i < N1; i++)
      TD[i + j * N1] = FD[j + i * N2];
}


/*************************************************************************
 *
 * 函数名称：
 *   WALSH()
 *
 * 参数:
 *   double * f        - 指向时域值的指针
 *   double * F        - 指向频域值的指针
 *   r            －2的幂数
 *   pBuffer        －请提供 2^r * sizeof(double) 大小的缓冲区
 *
 * 返回值:
 *   无。
 *
 * 说明:
 *   该函数用来实现快速沃尔什-哈达玛变换。
 *
 ************************************************************************/
void WALSH(const double *f, double *F, const int r, double *pBuffer) {
  int i, j, k, p;

  // 沃尔什-哈达玛变换点数
  int N = 1 << r;

  // 分配运算所需的数组
  double *X1 = pBuffer;
  double *X2 = pBuffer + N;
  double *X;

  // 将时域点写入数组X1
  memcpy(X1, f, sizeof(double) * N);

  // 蝶形运算
  for(k = 0; k < r; k++) {
    for(j = 0; j < 1<<k; j++) {
      int bfsize = 1 << (r-k);
      for(i = 0; i < bfsize / 2; i++) {
        p = j * bfsize;
        X2[i + p] = X1[i + p] + X1[i + p + bfsize / 2];
        X2[i + p + bfsize / 2] = X1[i + p] - X1[i + p + bfsize / 2];
      }
    }
    X = X1;
    X1 = X2;
    X2 = X;
  }

  // 调整系数
  for(j = 0; j < N; j++) {
    p = 0;
    for(i = 0; i < r; i++) {
      if (j & (1<<i)) {
        p += 1 << (r-i-1);
      }
    }
    F[j] = X1[p] / N;
  }
}

/*************************************************************************
 *
 * 函数名称：
 *   WALSH_2D()
 *
 * 参数:
 *   double * f        - 指向时域值的指针
 *   double * F        - 指向频域值的指针
 *   r1            －2 的幂数宽度
 *   r2            －2 的幂数高度
 *   pBuffer        －请提供 2^max{r1,r2} * sizeof(double) 大小的缓冲区
 *
 * 返回值:
 *   无。
 *
 * 说明:
 *   该函数用来实现快速二维沃尔什-哈达玛变换。
 *
 ************************************************************************/
/*    错误算法
void WALSH_2D(const double *f, double *F, int r1, int r2, double *pBuffer) {
  int i, j;

  int N1 = 1 << r1;
  int N2 = 1 << r2;

  // 对第一维度进行沃尔什-哈达玛变换
  for (j = 0; j < N2; j++)
    WALSH(&f[j * N1], &F[j * N1], r1, pBuffer);

  // 翻转以便进行第二维变换
  for (i = 0; i < N1; i++)
    for (j = 0; j < N2; j++)
      f[j + i * N2] = F[i + j * N1];

  // 对第二维度进行傅立叶变换
  for (i = 0; i < N1; i++)
    WALSH(&f[i * N2], &F[i * N2], r2, pBuffer);

  // 再次翻转得到最终二维变换结果
  memcpy(f, F, N1 * N2 * sizeof(double));
  for (j = 0; j < N2; j++)
    for (i = 0; i < N1; i++)
      F[i + j * N1] = f[j + i * N2];
}
*/


/*************************************************************************
 *
 * 函数名称：
 *   IWALSH()
 *
 * 参数:
 *   double * F        - 指向频域值的指针
 *   double * f        - 指向时域值的指针
 *   r            －2的幂数
 *
 * 返回值:
 *   无。
 *
 * 说明:
 *   该函数用来实现快速沃尔什-哈达玛反变换。
 *
 ************************************************************************/

void IWALSH(const double *F, double *f, const int r)
{
  // 沃尔什-哈达玛变换点数
  long  count;
  
  // 循环变量
  int    i,j,k;
  
  // 中间变量
  int    bfsize,p;
  
  double *X1,*X2,*X;
  
  // 计算快速沃尔什变换点数
  count = 1 << r;
  
  // 分配运算所需的数组
  X1 = new double[count];
  X2 = new double[count];
  
  // 将频域点写入数组X1
  memcpy(X1, F, sizeof(double) * count);
  
  // 蝶形运算
  for(k = 0; k < r; k++)
  {
    for(j = 0; j < 1<<k; j++)
    {
      bfsize = 1 << (r-k);
      for(i = 0; i < bfsize / 2; i++)
      {
        p = j * bfsize;
        X2[i + p] = X1[i + p] + X1[i + p + bfsize / 2];
        X2[i + p + bfsize / 2] = X1[i + p] - X1[i + p + bfsize / 2];
      }
    }
    
    // 互换X1和X2  
    X = X1;
    X1 = X2;
    X2 = X;
  }
  
  // 调整系数
  for(j = 0; j < count; j++)
  {
    p = 0;
    for(i = 0; i < r; i++)
    {
      if (j & (1<<i))
      {
        p += 1 << (r-i-1);
      }
    }

    f[j] = X1[p] / count;
  }
  
  // 释放内存
  delete X1;
  delete X2;
}



/*************************************************************************
 *
 * 函数名称：
 *   DCT()
 *
 * 参数:
 *   double * f        - 指向时域值的指针
 *   double * F        - 指向频域值的指针
 *   r            －2的幂数
 *
 * 返回值:
 *   无。
 *
 * 说明:
 *   该函数用来实现快速离散余弦变换。该函数利用2N点的快速付立叶变换
 * 来实现离散余弦变换。
 *
 ************************************************************************/
void DCT(const double *f, double *F, const int r) {
/*
  // 离散余弦变换点数
  long count;

  // 循环变量
  int i;

  // 中间变量
  double  dTemp;

  std::complex<double> *X;

  // 计算离散余弦变换点数
  count = 1<<r;

  std::complex<double> *buffer = new std::complex<double>[count * 6];

  // 分配内存
  X = new std::complex<double>[count*2];

  // 赋初值为0
  memset(X, 0, sizeof(std::complex<double>) * count * 2);

  // 将时域点写入数组X
  for(i=0;i<count;i++) {
    X[i] =std::complex<double> (f[i], 0);
  }

  // 调用快速付立叶变换
  FFT(X, X, r+1, buffer);

  // 调整系数
  dTemp = 1/sqrt((double)count);

  // 求F[0]
  F[0] = X[0].real() * dTemp;

  dTemp *= sqrt(2.0l);

  // 求F[u]  
  for(i = 1; i < count; i++) {
    F[i]=(X[i].real() * cos(i*M_PI/(count*2)) + X[i].imag() * sin(i*M_PI/(count*2))) * dTemp;
  }

  // 释放内存
  delete[] buffer;
  delete[] X;
*/
}





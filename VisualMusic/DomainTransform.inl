
// DomainTransform.inl

// ���������������͸���
template <typename type>
inline void dscopy(type *dest, type *src, int n, int step = 1){
	register type *pdestEnd = dest + n * step;
	while (dest < pdestEnd){
		*dest = *src;

		dest += step;
		src += step;
	}
}

// ��������С���任
inline void HWT_0(double *f, double *F, int r, int step = 1){
	register int Nd2 = 1 << (r-1);
	register double *pf = f;
	register double *pfEnd = pf + (Nd2 << 1) * step;
	register double *pFL = F;
	register double *pFH = pFL + Nd2 * step;;
	while (pf < pfEnd){
		register double d1 = *pf;
		register double d2 = *(pf + step);

		*pFL = (d1 + d2) / 2;
		*pFH = (d1 - d2) / 2;

		pf += step << 1;
		pFL += step;
		pFH += step;
	}
}

// ��������С�����任
inline void IHWT_0(double *F, double *f, int r, int step = 1){
	register int Nd2 = 1 << (r-1);
	register double *pf = f;
	register double *pfEnd = pf + (Nd2 << 1) * step;
	register double *pFL = F;
	register double *pFH = pFL + Nd2 * step;;
	while (pf < pfEnd){
		register double D1 = *pFL;
		register double D2 = *pFH;

		*pf = D1 + D2;
		*(pf+step) = D1 - D2;

		pf += step << 1;
		pFL += step;
		pFH += step;
	}
}


/*************************************************************************
 *
 * �������ƣ�
 *   HWT()
 *
 * ����:
 *   double * f				- ָ��ʱ��ֵ��ָ��
 *   double * F				- ָ��Ƶ��ֵ��ָ��
 *   r						��2 ������
 *
 * ����ֵ:
 *   �ޡ�
 *
 * ˵��:
 *   �ú�������ʵ��һά����С���任��
 *
 ************************************************************************/
inline void HWT(double *f, double *F, register int r){
	for (;r;r--){
		HWT_0(f, F, r);
		memcpy(f, F, ((intptr_t)1<<r) * sizeof(double));
	}
}

/*************************************************************************
 *
 * �������ƣ�
 *   IHWT()
 *
 * ����:
 *   double * F				- ָ��Ƶ��ֵ��ָ��
 *   double * f				- ָ��ʱ��ֵ��ָ��
 *   r						��2 ������
 *
 * ����ֵ:
 *   �ޡ�
 *
 * ˵��:
 *   �ú�������ʵ��һά����С�����任��
 *
 ************************************************************************/
inline void IHWT(double *F, double *f, register int r){
	for (register int i = 1; i <= r; i++){
		IHWT_0(F, f, i);
		memcpy(F, f, ((intptr_t)1<<i) * sizeof(double));
	}
}



inline void DFT(std::complex<double> *TD, std::complex<double> *FD, int N){
	int k, n;

	// ������������洢��
	std::complex<double> *W  = new std::complex<double>[N * N];

	// �����Ȩϵ��
	for(k = 0; k < N; k++){
		for(n = 0; n < N; n++){
			int i = k * n;
			double angle = - M_PI * 2 * i / N;
			W[i] = std::complex<double>(cos(angle), sin(angle));
		}
	}

	// ����DFT��ʽ���б任
	for (k = 0; k < N; k++){
		std::complex<double> fdi(0, 0);
		for (n = 0; n < N; n++){
			fdi += TD[n] * W[k * n];
		}
		FD[k] = fdi;
	}

	// �ͷ��ڴ�
	delete[] W;
}

inline void IDFT(std::complex<double> *FD, std::complex<double> *TD, int N){
	int k, n;

	// ������������洢��
	std::complex<double> *IW  = new std::complex<double>[N * N];

	// �����Ȩϵ��
	for(k = 0; k < N; k++){
		for(n = 0; n < N; n++){
			int i = k * n;
			double angle = M_PI * 2 * i / N;
			IW[i] = std::complex<double>(cos(angle), sin(angle));
		}
	}

	// ����DFT��ʽ���б任
	for (k = 0; k < N; k++){
		std::complex<double> fdi(0, 0);
		for (n = 0; n < N; n++){
			fdi += FD[n] * IW[k * n];
		}
		TD[k] = fdi / (double)N;
	}

	// �ͷ��ڴ�
	delete[] IW;
}


inline CFFT::CFFT(register int r):
	R(r),
	N(1<<R)
{
	assert(r >= 0 && r < 32);

	if (!all_buf[r]){
		register std::complex<double> *W =
			all_buf[r] = new std::complex<double>[N * 3];

		// �����Ȩϵ��
		for(register int i = 0; i < N; i++){
			register double angle = M_PI * 2 * i / N;
			W[i] = std::complex<double>(cos(angle), -sin(angle));
		}
	}

	n_Object++;
}


inline CFFT::~CFFT(){
	n_Object--;

	if (n_Object == 0)
		for (int i = 0; i < 32; i++)
			if (all_buf[i]){
				delete all_buf[i];
				all_buf[i] = NULL;
			}
}


template <class TYPE>
inline void CFFT::FFT(TYPE *TD, std::complex<double> *FD){

	// �����м����
	register std::complex<double> *X1 = all_buf[R] + N;
	register std::complex<double> *X2 = X1 + N;

	// ��ʱ���д��X1
	register TYPE *src_end = TD + N;
	register TYPE *src = TD;
	register std::complex<double> *dst = X1;
	while (src < src_end)
		*(dst++) = *(src++);

	// �����㷨
	register std::complex<double> *W = all_buf[R];
	for(register int k = 0; k < R; k++){
		register int bfsize = POWER_2[R - k - 1];
		for(register int j = 0; j < POWER_2[k]; j++){
			register int p = j * POWER_2[R - k];
			for(register int i = 0; i < bfsize; i++){
				X2[i + p] = X1[i + p] + X1[i + p + bfsize];
				X2[i + p + bfsize] = (X1[i + p] - X1[i + p + bfsize]) * W[i * POWER_2[k]];
			}
		}

		// ���� X1 �� X2
		register std::complex<double> *X  = X1;
		X1 = X2;
		X2 = X;
	}

	// ��������
	for(register int j = 0; j < N; j++){
		register int p = 0;
		for(register int i = 0; i < R; i++)
			if (j & POWER_2[i])
				p += POWER_2[R - i - 1];

		FD[j] = X1[p];
	}
}

template <class TYPE, class FTYPE>
inline void CFFT::FFT(TYPE *TD, FTYPE *FD){

	// �����м����
	register std::complex<double> *X1 = all_buf[R] + N;
	register std::complex<double> *X2 = X1 + N;

	// ��ʱ���д��X1
	register TYPE *src_end = TD + N;
	register TYPE *src = TD;
	register std::complex<double> *dst = X1;
	while (src < src_end)
		*(dst++) = *(src++);

	// �����㷨
	register std::complex<double> *W = all_buf[R];
	for(register int k = 0; k < R; k++){
		register int bfsize = POWER_2[R - k - 1];
		for(register int j = 0; j < POWER_2[k]; j++){
			register int p = j * POWER_2[R - k];
			for(register int i = 0; i < bfsize; i++){
				X2[i + p] = X1[i + p] + X1[i + p + bfsize];
				X2[i + p + bfsize] = (X1[i + p] - X1[i + p + bfsize]) * W[i * POWER_2[k]];
			}
		}

		// ���� X1 �� X2
		register std::complex<double> *X  = X1;
		X1 = X2;
		X2 = X;
	}

	// ��������
	for(register int j = 0; j < N; j++){
		register int p = 0;
		for(register int i = 0; i < R; i++)
			if (j & POWER_2[i])
				p += POWER_2[R - i - 1];

		FD[j] = abs(X1[p]);
	}
}


inline void CFFT::IFFT(std::complex<double> *FD, std::complex<double> *TD){

	// �����м����
	register std::complex<double> *X1 = all_buf[R] + N;
	register std::complex<double> *X2 = X1 + N;

	// ��Ƶ���д��X1
	register std::complex<double> *src_end = FD + N;
	register std::complex<double> *src = FD;
	register std::complex<double> *dst = X1;
	while (src < src_end)
		*(dst++) = *(src++);

	// �����㷨
	register std::complex<double> *W = all_buf[R];
	for(register int k = 0; k < R; k++){
		register int bfsize = POWER_2[R - k - 1];
		for(register int j = 0; j < POWER_2[k]; j++){
			register int p = j * POWER_2[R - k];
			for(register int i = 0; i < bfsize; i++){
				X2[i + p] = X1[i + p] + X1[i + p + bfsize];
				X2[i + p + bfsize] = (X1[i + p] - X1[i + p + bfsize]) * conj(W[i * POWER_2[k]]);
			}
		}

		// ���� X1 �� X2
		register std::complex<double> *X  = X1;
		X1 = X2;
		X2 = X;
	}

	// ��������
	for(register int j = 0; j < N; j++){
		register int p = 0;
		for(register int i = 0; i < R; i++)
			if (j & POWER_2[i])
				p += POWER_2[R - i - 1];

		TD[j] = X1[p] / (double)N;
	}
}


inline void CFFT_2D::FFT_2D(std::complex<double> *TD, std::complex<double> *FD){
	int i, j;

	int N1 = 1 << R1;
	int N2 = 1 << R2;

	// �Ե�һά�Ƚ��и���Ҷ�任
	for (j = 0; j < N2; j++)
		fft1.FFT(&TD[j * N1], &FD[j * N1]);

	// ��ת�Ա���еڶ�ά�任
	for (i = 0; i < N1; i++)
		for (j = 0; j < N2; j++)
			TD[j + i * N2] = FD[i + j * N1];

	// �Եڶ�ά�Ƚ��и���Ҷ�任
	for (i = 0; i < N1; i++)
		fft2.FFT(&TD[i * N2], &FD[i * N2]);

	// �ٴη�ת�õ����ն�ά�任���
	memcpy(TD, FD, N1 * N2 * sizeof(std::complex<double>));
	for (j = 0; j < N2; j++)
		for (i = 0; i < N1; i++)
			FD[i + j * N1] = TD[j + i * N2];
}

// ��ά����Ҷ���任 ����Ҫ�ṩ 3 * 2^max(r1,r2) * sizeof(std::complex<double>) ��С�Ļ�������
inline void CFFT_2D::IFFT_2D(std::complex<double> *FD, std::complex<double> *TD){
	int i, j;

	int N1 = 1 << R1;
	int N2 = 1 << R2;

	// �Ե�һά�Ƚ��и���Ҷ���任
	for (j = 0; j < N2; j++)
		fft1.IFFT(&FD[j * N1], &TD[j * N1]);

	// ��ת�Ա���еڶ�ά�任
	for (i = 0; i < N1; i++)
		for (j = 0; j < N2; j++)
			FD[j + i * N2] = TD[i + j * N1];

	// �Եڶ�ά�Ƚ��и���Ҷ���任
	for (i = 0; i < N1; i++)
		fft2.IFFT(&FD[i * N2], &TD[i * N2]);

	// �ٴη�ת�õ����ն�ά�任���
	memcpy(FD, TD, N1 * N2 * sizeof(std::complex<double>));
	for (j = 0; j < N2; j++)
		for (i = 0; i < N1; i++)
			TD[i + j * N1] = FD[j + i * N2];
}

// ��ά����С���任
inline void HWT_2D(double *f, double *F, int r1, int r2){
	int i, j;
	int step = 1 << r1;

	while (r1 && r2){
		int N1 = 1 << r1;
		int N2 = 1 << r2;

		if (r1){
			// �Ե�һά�Ƚ���һά��������С���任
			for (j = 0; j < N2; j++)
				HWT_0(&f[j * step], &F[j * step], r1);
			for (j = 0; j < N2; j++)
				dscopy(&f[j * step], &F[j * step], N1);
			r1--;
		}

		if (r2){
			// �Եڶ�ά�Ƚ���һά��������С���任
			for (i = 0; i < N1; i++)
				HWT_0(&f[i], &F[i], r2, step);
			for (i = 0; i < N1; i++)
				dscopy(&f[i], &F[i], N2, step);
			r2--;
		}
	}
}


/*************************************************************************
 *
 * �������ƣ�
 *   WALSH()
 *
 * ����:
 *   double * f				- ָ��ʱ��ֵ��ָ��
 *   double * F				- ָ��Ƶ��ֵ��ָ��
 *   r						��2������
 *   pBuffer				�����ṩ 2^r * sizeof(double) ��С�Ļ�����
 *
 * ����ֵ:
 *   �ޡ�
 *
 * ˵��:
 *   �ú�������ʵ�ֿ����ֶ�ʲ-������任��
 *
 ************************************************************************/
inline void WALSH(double *f, double *F, int r, double *pBuffer){
	int i, j, k, p;

	// �ֶ�ʲ-������任����
	int N = 1 << r;

	// �����������������
	double *X1 = pBuffer;
	double *X2 = pBuffer + N;
	double *X;

	// ��ʱ���д������X1
	memcpy(X1, f, sizeof(double) * N);

	// ��������
	for(k = 0; k < r; k++){
		for(j = 0; j < 1<<k; j++){
			int bfsize = 1 << (r-k);
			for(i = 0; i < bfsize / 2; i++){
				p = j * bfsize;
				X2[i + p] = X1[i + p] + X1[i + p + bfsize / 2];
				X2[i + p + bfsize / 2] = X1[i + p] - X1[i + p + bfsize / 2];
			}
		}
		X = X1;
		X1 = X2;
		X2 = X;
	}

	// ����ϵ��
	for(j = 0; j < N; j++){
		p = 0;
		for(i = 0; i < r; i++){
			if (j & (1<<i)){
				p += 1 << (r-i-1);
			}
		}
		F[j] = X1[p] / N;
	}
}

/*************************************************************************
 *
 * �������ƣ�
 *   WALSH_2D()
 *
 * ����:
 *   double * f				- ָ��ʱ��ֵ��ָ��
 *   double * F				- ָ��Ƶ��ֵ��ָ��
 *   r1						��2 ���������
 *   r2						��2 �������߶�
 *   pBuffer				�����ṩ 2^max{r1,r2} * sizeof(double) ��С�Ļ�����
 *
 * ����ֵ:
 *   �ޡ�
 *
 * ˵��:
 *   �ú�������ʵ�ֿ��ٶ�ά�ֶ�ʲ-������任��
 *
 ************************************************************************/
/*		�����㷨
inline void WALSH_2D(double *f, double *F, int r1, int r2, double *pBuffer){
	int i, j;

	int N1 = 1 << r1;
	int N2 = 1 << r2;

	// �Ե�һά�Ƚ����ֶ�ʲ-������任
	for (j = 0; j < N2; j++)
		WALSH(&f[j * N1], &F[j * N1], r1, pBuffer);

	// ��ת�Ա���еڶ�ά�任
	for (i = 0; i < N1; i++)
		for (j = 0; j < N2; j++)
			f[j + i * N2] = F[i + j * N1];

	// �Եڶ�ά�Ƚ��и���Ҷ�任
	for (i = 0; i < N1; i++)
		WALSH(&f[i * N2], &F[i * N2], r2, pBuffer);

	// �ٴη�ת�õ����ն�ά�任���
	memcpy(f, F, N1 * N2 * sizeof(double));
	for (j = 0; j < N2; j++)
		for (i = 0; i < N1; i++)
			F[i + j * N1] = f[j + i * N2];
}
*/


/*************************************************************************
 *
 * �������ƣ�
 *   IWALSH()
 *
 * ����:
 *   double * F				- ָ��Ƶ��ֵ��ָ��
 *   double * f				- ָ��ʱ��ֵ��ָ��
 *   r						��2������
 *
 * ����ֵ:
 *   �ޡ�
 *
 * ˵��:
 *   �ú�������ʵ�ֿ����ֶ�ʲ-�����귴�任��
 *
 ************************************************************************/

inline void IWALSH(double *F, double *f, int r)
{
	// �ֶ�ʲ-������任����
	long	count;
	
	// ѭ������
	int		i,j,k;
	
	// �м����
	int		bfsize,p;
	
	double *X1,*X2,*X;
	
	// ��������ֶ�ʲ�任����
	count = 1 << r;
	
	// �����������������
	X1 = new double[count];
	X2 = new double[count];
	
	// ��Ƶ���д������X1
	memcpy(X1, F, sizeof(double) * count);
	
	// ��������
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
		
		// ����X1��X2  
		X = X1;
		X1 = X2;
		X2 = X;
	}
	
	// ����ϵ��
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
	
	// �ͷ��ڴ�
	delete X1;
	delete X2;
}



/*************************************************************************
 *
 * �������ƣ�
 *   DCT()
 *
 * ����:
 *   double * f				- ָ��ʱ��ֵ��ָ��
 *   double * F				- ָ��Ƶ��ֵ��ָ��
 *   r						��2������
 *
 * ����ֵ:
 *   �ޡ�
 *
 * ˵��:
 *   �ú�������ʵ�ֿ�����ɢ���ұ任���ú�������2N��Ŀ��ٸ���Ҷ�任
 * ��ʵ����ɢ���ұ任��
 *
 ************************************************************************/
inline void DCT(double *f, double *F, int r){
/*
	// ��ɢ���ұ任����
	long count;

	// ѭ������
	int i;

	// �м����
	double	dTemp;

	std::complex<double> *X;

	// ������ɢ���ұ任����
	count = 1<<r;

	std::complex<double> *buffer = new std::complex<double>[count * 6];

	// �����ڴ�
	X = new std::complex<double>[count*2];

	// ����ֵΪ0
	memset(X, 0, sizeof(std::complex<double>) * count * 2);

	// ��ʱ���д������X
	for(i=0;i<count;i++){
		X[i] =std::complex<double> (f[i], 0);
	}

	// ���ÿ��ٸ���Ҷ�任
	FFT(X, X, r+1, buffer);

	// ����ϵ��
	dTemp = 1/sqrt((double)count);

	// ��F[0]
	F[0] = X[0].real() * dTemp;

	dTemp *= sqrt(2.0l);

	// ��F[u]	
	for(i = 1; i < count; i++){
		F[i]=(X[i].real() * cos(i*M_PI/(count*2)) + X[i].imag() * sin(i*M_PI/(count*2))) * dTemp;
	}

	// �ͷ��ڴ�
	delete[] buffer;
	delete[] X;
*/
}



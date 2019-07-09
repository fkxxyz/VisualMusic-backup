
// AudioPCMAnalyser.h

#pragma once

#include <cfloat>

// ��С�񶯴�������������Ƶ��⣩
#define MIN_VIB_N	32.0

// ����񶯴���
#define MAX_VIB_N	128.0

// ��Сʱ��ֱ��ʣ����ٰ�һ���ӷֳɶ��ٷݣ����ﵽ��С�ֱ���ʱ�Զ������񶯴�����
#define MIN_TIME_RAT	1
#define MAX_TIME_RAT	50

// ̮������ֵ
#define MD_SCALE_UPPER	65536.0l

// ��һ�������Ҫ��������������ֵԽ���Ż��ϲ��ĵ�ԽС��Ч�ʸ���
#define MAX_N1	8

// ��ֵ������� DCT ��������
#define MAX_N	512

// ��ǰ����sin��cos��ֵ����Ч�ʣ���ֵ��ʾ��һ��Բ�ֳɶ��ٷݼ���
#define SIN_COS_RATE	65536

template <class TYPE>
class CAudioPCMAnalyser {

	CPCMStream<TYPE> *m_pcm;

	double *cos_div, *sin_div;

	// ���������Ż��������������
	TYPE *m_td_buf;
	TYPE *m_td[MAX_CHANNELS][32];

public:
	CAudioPCMAnalyser():m_pcm(NULL){}
	CAudioPCMAnalyser(CPCMStream<TYPE> *pcm):m_pcm(NULL){ Init(pcm); }

	~CAudioPCMAnalyser(){ Clear(); }

	void Clear(){
		if (!m_pcm) return;
		delete m_td_buf;
		delete cos_div;
	}

	void Init(
		CPCMStream<TYPE> *pcm
	){
		Clear();

		// ���ù̶�����
		m_pcm = pcm;

		// ���� cos �� sin �ķָ��Ż�����
		cos_div = new double [SIN_COS_RATE * 2];
		sin_div = cos_div + SIN_COS_RATE;
		for (int i = 0; i < SIN_COS_RATE; i++){
			double angle = (2 * M_PI) * ((double)i / SIN_COS_RATE);
			cos_div[i] = cos(angle);
			sin_div[i] = sin(angle);
		}

		// �������������Ŵ���
		int max_n = pcm->m_nSamples;
		int max_optimizeR = 0;
		while (max_n > MAX_N){
			max_n >>= 1;
			max_optimizeR++;
		}

		// ��������
		m_td_buf = new TYPE[pcm->m_nSamples * pcm->m_nChannels];
		TYPE *m_td_max_min = new TYPE[pcm->m_nSamples];
		TYPE *m_td_max = m_td_max_min, *m_td_min = m_td_max_min + pcm->m_nSamples / 2;
		for (int n = 0; n < pcm->m_nChannels; n++){
			register TYPE *p_src = pcm->m_Samples[n];
			register TYPE *p_src_end = p_src + pcm->m_nSamples;
			register TYPE *p_dst = m_td_buf;
			register TYPE *p_src_max = p_src;
			register TYPE *p_src_min = p_src;
			register TYPE *p_dest_max = m_td_max;
			register TYPE *p_dest_min = m_td_min;
			m_td[n][0] = p_src;
			while (p_src != p_src_end){
				p_src++;
				if (p_src == p_src_end) break;
				p_src++;

				register TYPE max = *(p_src_max++);
				if (*p_src_max > max) max = *p_src_max;
				*(p_dest_max++) = max;
				p_src_max++;

				register TYPE min = *(p_src_min++);
				if (*p_src_min < min) min = *p_src_min;
				*(p_dest_min++) = min;
				p_src_min++;

				*(p_dst++) = (max + min) / 2;
			}

			p_src = m_td_buf;
			for (int k = 1; k <= max_optimizeR; k++){
				m_td[n][k] = p_src;
				p_src_end = p_dst;
				p_src_max = m_td_max;
				p_src_min = m_td_min;
				p_dest_max = m_td_max;
				p_dest_min = m_td_min;
				while (p_src != p_src_end){
					p_src++;
					if (p_src == p_src_end) break;
					p_src++;

					register TYPE max = *(p_src_max++);
					if (*p_src_max > max) max = *p_src_max;
					*(p_dest_max++) = max;
					p_src_max++;

					register TYPE min = *(p_src_min++);
					if (*p_src_min < min) min = *p_src_min;
					*(p_dest_min++) = min;
					p_src_min++;

					*(p_dst++) = (max + min) / 2;
				}
			}
		}
		delete m_td_max_min;
	}

	inline double dconv(TYPE *simple, double *base, int N, int N_end, double vib_n, double N1){
		double v = 0;
		int i_end = N < N_end ? N : N_end;
		for (int i = 0; i < i_end; i++)
			v += simple[i] * base[(int)((double)i/N1 * SIN_COS_RATE) % SIN_COS_RATE];
		return v;
	}

	inline double sqr(double x){ return x*x; }

	inline double theo(double a, double b){ return sqrt(a*a+b*b); }

	inline double dct_func(TYPE *simple, int N, int N_end, double vib_n, double N1){
		return theo(dconv(simple, cos_div, N, N_end, vib_n, N1), dconv(simple, sin_div, N, N_end, vib_n, N1))/(N/2);
	}

	static int __cdecl FuncCompare(const void *p1, const void *p2){
		return *(double *)p1 - *(double *)p2 > 0 ? 1 : -1;
	}

	inline double sample_pos_dct(int sample_pos, int iChannel, double freq, double vib_n){
		// ����Ҫ�� DCT ��ʽ���ǵ�������
		double N1 = m_pcm->m_nSamplesPerSec / freq;
		int N = (int)(vib_n * N1);

		// ȷ��������������β
		int N_end = N;
		if (sample_pos + N > m_pcm->m_nSamples)
			N_end = m_pcm->m_nSamples - sample_pos;
		if (N_end < 0) return 0l;

		int r = 0;
		// �����Ż��������Ż�
		while (N > MAX_N){
			N1 /= 2;
			N >>= 1;
			N_end >>= 1;
			sample_pos >>= 1;
			r++;
		}

		TYPE *s = m_td[iChannel][r] + sample_pos;
		return dct_func(s, N, N_end, vib_n, N1);
	}

	inline double GetAmpOfFreq(int iMilliseconds, int iChannel, double freq){
		assert(iMilliseconds >= 0);
		assert(freq > 0);

		// �����ʱ���Ӧ������λ��
		int sample_pos = (int)((double)iMilliseconds / 1000 * m_pcm->m_nSamplesPerSec);

		// ȷ���񶯴�����ȷ�������������õ���Сʱ��ֱ���
		double vib_n = freq / MAX_TIME_RAT;

		// ȷ���񶯴��������������õ�����񶯴���
		if (vib_n > MAX_VIB_N) vib_n = MAX_VIB_N;
		if (vib_n < MIN_VIB_N) vib_n = MIN_VIB_N;

		//if (vib_n < 1)
		//	vib_n = 1;

		return sample_pos_dct(sample_pos, iChannel, freq, vib_n);

if (0){
		if (vib_n < MIN_VIB_N)
			return
				sample_pos_dct(sample_pos, iChannel, freq, MIN_VIB_N) +
				sample_pos_dct(sample_pos, iChannel, freq, vib_n) * (0.5l + vib_n / 2 / MIN_VIB_N);
		else
			return sample_pos_dct(sample_pos, iChannel, freq, vib_n);
}

		// ��һ������������

		// ���ֱ���������񶯵Ĵ���
		int time_pix_vib_n = (int)(freq / MAX_TIME_RAT);
		if (time_pix_vib_n == 0) time_pix_vib_n = 1;

		// ����Ҫ�� DCT ��ʽ���ǵ�������
		double N1 = m_pcm->m_nSamplesPerSec / freq;
		int N = (int)(vib_n * N1);

		// ȷ��������������β
		int N_end = N;
		if (sample_pos + N > m_pcm->m_nSamples)
			N_end = m_pcm->m_nSamples - sample_pos;
		if (N_end < 0) return 0l;

		int r = 0;
		// �����Ż��������Ż�
		while (N1 > MAX_N1){
			N1 /= 2;
			N >>= 1;
			N_end >>= 1;
			sample_pos >>= 1;
			r++;
		}

		TYPE *s = m_td[iChannel][r] + sample_pos;

		// ������
		double head_p_value = 0;
		double max_p_value = head_p_value;
		int max_p_index = 0;
		double avg_p_value = 0;
		int sum_n = 0;
		int sum_n_head = 0;
		double value_p[(int)(MAX_VIB_N/2)];
		for (int vib_n_pos = 0; vib_n_pos+2 < vib_n; vib_n_pos += 2){
			double value = dct_func(s + (int)(vib_n_pos * N1), (int)(2 * N1), N_end - (int)(vib_n_pos * N1), 2, N1);

			if (vib_n_pos < time_pix_vib_n){
				head_p_value += value;
				sum_n_head++;
			}
			/*
			int n = (int)(2 * N1);
			double cos_result = 0, sin_result = 0;
			int i_end = n < N_end ? n : N_end;
			for (int i = 0; i < i_end; i++){
				cos_result += s[i + (int)(vib_n_pos * N1)] * cos_div[(int)(2 * SIN_COS_RATE * (double)i / n) % SIN_COS_RATE];
				sin_result += s[i + (int)(vib_n_pos * N1)] * sin_div[(int)(2 * SIN_COS_RATE * (double)i / n) % SIN_COS_RATE];
			}
			double value = sqrt(cos_result * cos_result + sin_result * sin_result)/(n/2);
			*/

			if (value > max_p_value){
				max_p_value = value;
				max_p_index = vib_n_pos>>1;
			}

			avg_p_value += value;
			value_p[sum_n++] = value;
		}
		avg_p_value /= sum_n;
		head_p_value /= sum_n_head;
		double max = 0;
		for (int i = 0; i < sum_n; i++){
			double v = (i+1) * value_p[i];
			if (v > max){
				max = v;
				max_p_index = i;
			}
		}
		qsort(value_p, sum_n, sizeof(double), FuncCompare);
		double med_p_value = value_p[sum_n / 2];
		//max_p_value = value_p[sum_n * 7 / 8];

		double value = dct_func(s, (int)(vib_n * N1), N_end, vib_n, N1);
		//if (head_p_value > max_p_value) head_p_value = max_p_value;
		double result = value * (head_p_value / max_p_value);
		if (_isnan(result)) result = 0;
		return result;

		if (0){
			// �� 2���񶯿�ʼ���
			double max_value = max_p_value;
			double last_value = max_value;
			int monotonicity = 0;
			int v_n;
			for (v_n = 4; v_n < vib_n; v_n *= 2){
				double value = dct_func(s, (int)(v_n * N1), N_end, v_n, N1);

				if (value > max_value)
					max_value = value;

				double sc = value / last_value;
				if (sc > 0){
					if (monotonicity == -1)
						break;
					monotonicity = 1;
				} else {
					if (monotonicity == 1)
						break;
					monotonicity = -1;
				}

				last_value = value;
			}

			// �����ʱ���Ӧ������λ��
			//sample_pos = (int)((double)iMilliseconds / 1000 * m_pcm->m_nSamplesPerSec);

			// ȷ���񶯴�����ȷ�������������õ���Сʱ��ֱ���
			//vib_n = freq / MAX_TIME_RAT;

			// ȷ���񶯴��������������õ�����񶯴���
			//if (vib_n > MAX_VIB_N) vib_n = MAX_VIB_N;

			//if (vib_n < 1) vib_n = 1;

			//return sample_pos_dct(sample_pos, iChannel, freq, vib_n);

			if (head_p_value > max_p_value) head_p_value = max_p_value;
			double result = last_value * (head_p_value / max_p_value);
			if (_isnan(result)) result = 0;
			if (result > m_pcm->m_max_amplitude) result = m_pcm->m_max_amplitude;
			//result = sample_pos_dct(sample_pos, iChannel, freq, vib_n) / 8;
			return result;
		}
	}

	inline double GetAmpOfFreq(int iMilliseconds, double freq){
		double sum = 0;
		for (int i = 0; i < m_pcm->m_nChannels; i++)
			sum += GetAmpOfFreq(iMilliseconds, i, freq);
		return sum / m_pcm->m_nChannels;
	}
};


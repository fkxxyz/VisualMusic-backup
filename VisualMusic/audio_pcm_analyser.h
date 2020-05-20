
// audio_pcm_analyser.h

#pragma once

#include <cfloat>

// ��С�񶯴�������������Ƶ��⣩
#define MIN_VIB_N  32.0

// ����񶯴���
#define MAX_VIB_N  128.0

// ��Сʱ��ֱ��ʣ����ٰ�һ���ӷֳɶ��ٷݣ����ﵽ��С�ֱ���ʱ�Զ������񶯴�����
#define MIN_TIME_RAT  1
#define MAX_TIME_RAT  50

// ̮������ֵ
#define MD_SCALE_UPPER  65536.0l

// ��һ�������Ҫ��������������ֵԽ���Ż��ϲ��ĵ�ԽС��Ч�ʸ���
#define MAX_N1  8

// ��ֵ������� DCT ��������
#define MAX_N  512

// ��ǰ����sin��cos��ֵ����Ч�ʣ���ֵ��ʾ��һ��Բ�ֳɶ��ٷݼ���
#define SIN_COS_RATE  65536

template <class TYPE>
class CAudioPCMAnalyser {
 public:
  CAudioPCMAnalyser():m_pcm_(NULL) {}
  CAudioPCMAnalyser(CPCMStream<TYPE> *pcm):m_pcm_(NULL) { Init(pcm); }

  ~CAudioPCMAnalyser() { Clear(); }

  void Clear() {
    if (!m_pcm_) return;
    delete m_td_buf_;
    delete cos_div_;
  }

  void Init(
    CPCMStream<TYPE> *pcm
  ) {
    Clear();

    // ���ù̶�����
    m_pcm_ = pcm;

    // ���� cos �� sin �ķָ��Ż�����
    cos_div_ = new double [SIN_COS_RATE * 2];
    sin_div_ = cos_div_ + SIN_COS_RATE;
    for (int i = 0; i < SIN_COS_RATE; i++) {
      double angle = (2 * M_PI) * ((double)i / SIN_COS_RATE);
      cos_div_[i] = cos(angle);
      sin_div_[i] = sin(angle);
    }

    // �������������Ŵ���
    int max_n = pcm->m_num_samples_;
    int max_optimizeR = 0;
    while (max_n > MAX_N) {
      max_n >>= 1;
      max_optimizeR++;
    }

    // ��������
    m_td_buf_ = new TYPE[pcm->m_num_samples_ * pcm->m_num_channels_];
    TYPE *m_td_max_min = new TYPE[pcm->m_num_samples_];
    TYPE *m_td_max =
      m_td_max_min, *m_td_min = m_td_max_min + pcm->m_num_samples_ / 2;
    for (int n = 0; n < pcm->m_num_channels_; n++) {
      register TYPE *p_src = pcm->m_samples_[n];
      register TYPE *p_src_end = p_src + pcm->m_num_samples_;
      register TYPE *p_dst = m_td_buf_;
      register TYPE *p_src_max = p_src;
      register TYPE *p_src_min = p_src;
      register TYPE *p_dest_max = m_td_max;
      register TYPE *p_dest_min = m_td_min;
      m_td_[n][0] = p_src;
      while (p_src != p_src_end) {
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

      p_src = m_td_buf_;
      for (int k = 1; k <= max_optimizeR; k++) {
        m_td_[n][k] = p_src;
        p_src_end = p_dst;
        p_src_max = m_td_max;
        p_src_min = m_td_min;
        p_dest_max = m_td_max;
        p_dest_min = m_td_min;
        while (p_src != p_src_end) {
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

  inline double dconv(const TYPE *simple, const double *base, const int n,
                      const int n_end, const double vib_n, const double n1) {
    double v = 0;
    int i_end = n < n_end ? n : n_end;
    for (int i = 0; i < i_end; i++)
      v += simple[i] * base[(int)((double)i/n1 * SIN_COS_RATE) % SIN_COS_RATE];
    return v;
  }

  inline double sqr(const double x) { return x*x; }

  inline double theo(const double a, const double b) { return sqrt(a*a+b*b); }

  inline double dct_func(const TYPE *simple, const int n, const int n_end,
                         const double vib_n, const double n1) {
    return theo(dconv(simple, cos_div_, n, n_end, vib_n, n1),
                dconv(simple, sin_div_, n, n_end, vib_n, n1))/(n/2);
  }

  static int __cdecl FuncCompare(const void *p1, const void *p2) {
    return *(double *)p1 - *(double *)p2 > 0 ? 1 : -1;
  }

  inline double sample_pos_dct(const int sample_pos, const int channel,
                               const double freq, const double vib_n) {
    // ����Ҫ�� DCT ��ʽ���ǵ�������
    double n1 = m_pcm_->m_num_samples_per_sec_ / freq;
    int n = (int)(vib_n * n1);

    // ȷ��������������β
    int n_end = n;
    if (sample_pos + n > m_pcm_->m_num_samples_)
      n_end = m_pcm_->m_num_samples_ - sample_pos;
    if (n_end < 0) return 0l;

    int r = 0;
    int s_pos = sample_pos;
    // �����Ż��������Ż�
    while (n > MAX_N) {
      n1 /= 2;
      n >>= 1;
      n_end >>= 1;
      s_pos >>= 1;
      r++;
    }

    TYPE *s = m_td_[channel][r] + s_pos;
    return dct_func(s, n, n_end, vib_n, n1);
  }

  inline double GetAmpOfFreq(const int milliseconds,
                             const int channel, const double freq) {
    assert(milliseconds >= 0);
    assert(freq > 0);

    // �����ʱ���Ӧ������λ��
    int sample_pos = (int)(
        (double)milliseconds / 1000 * m_pcm_->m_num_samples_per_sec_);

    // ȷ���񶯴�����ȷ�������������õ���Сʱ��ֱ���
    double vib_n = freq / MAX_TIME_RAT;

    // ȷ���񶯴��������������õ�����񶯴���
    if (vib_n > MAX_VIB_N) vib_n = MAX_VIB_N;
    if (vib_n < MIN_VIB_N) vib_n = MIN_VIB_N;

    //if (vib_n < 1)
    //  vib_n = 1;

    return sample_pos_dct(sample_pos, channel, freq, vib_n);

if (0) {
    if (vib_n < MIN_VIB_N)
      return
        sample_pos_dct(sample_pos, channel, freq, MIN_VIB_N) +
        sample_pos_dct(sample_pos, channel, freq, vib_n) * (0.5l + vib_n / 2 / MIN_VIB_N);
    else
      return sample_pos_dct(sample_pos, channel, freq, vib_n);
}

    // ��һ������������

    // ���ֱ���������񶯵Ĵ���
    int time_pix_vib_n = (int)(freq / MAX_TIME_RAT);
    if (time_pix_vib_n == 0) time_pix_vib_n = 1;

    // ����Ҫ�� DCT ��ʽ���ǵ�������
    double n1 = m_pcm_->m_num_samples_per_sec_ / freq;
    int N = (int)(vib_n * n1);

    // ȷ��������������β
    int n_end = N;
    if (sample_pos + N > m_pcm_->m_num_samples_)
      n_end = m_pcm_->m_num_samples_ - sample_pos;
    if (n_end < 0) return 0l;

    int r = 0;
    // �����Ż��������Ż�
    while (n1 > MAX_N1) {
      n1 /= 2;
      N >>= 1;
      n_end >>= 1;
      sample_pos >>= 1;
      r++;
    }

    TYPE *s = m_td_[channel][r] + sample_pos;

    // ������
    double head_p_value = 0;
    double max_p_value = head_p_value;
    int max_p_index = 0;
    double avg_p_value = 0;
    int sum_n = 0;
    int sum_n_head = 0;
    double value_p[(int)(MAX_VIB_N/2)];
    for (int vib_n_pos = 0; vib_n_pos+2 < vib_n; vib_n_pos += 2) {
      double value = dct_func(s + (int)(vib_n_pos * n1), (int)(2 * n1),
                              n_end - (int)(vib_n_pos * n1), 2, n1);

      if (vib_n_pos < time_pix_vib_n) {
        head_p_value += value;
        sum_n_head++;
      }
      /*
      int n = (int)(2 * n1);
      double cos_result = 0, sin_result = 0;
      int i_end = n < n_end ? n : n_end;
      for (int i = 0; i < i_end; i++) {
        cos_result += s[i + (int)(vib_n_pos * n1)] * cos_div_[(int)(2 * SIN_COS_RATE * (double)i / n) % SIN_COS_RATE];
        sin_result += s[i + (int)(vib_n_pos * n1)] * sin_div_[(int)(2 * SIN_COS_RATE * (double)i / n) % SIN_COS_RATE];
      }
      double value = sqrt(cos_result * cos_result + sin_result * sin_result)/(n/2);
      */

      if (value > max_p_value) {
        max_p_value = value;
        max_p_index = vib_n_pos>>1;
      }

      avg_p_value += value;
      value_p[sum_n++] = value;
    }
    avg_p_value /= sum_n;
    head_p_value /= sum_n_head;
    double max = 0;
    for (int i = 0; i < sum_n; i++) {
      double v = (i+1) * value_p[i];
      if (v > max) {
        max = v;
        max_p_index = i;
      }
    }
    qsort(value_p, sum_n, sizeof(double), FuncCompare);
    double med_p_value = value_p[sum_n / 2];
    //max_p_value = value_p[sum_n * 7 / 8];

    double value = dct_func(s, (int)(vib_n * n1), n_end, vib_n, n1);
    //if (head_p_value > max_p_value) head_p_value = max_p_value;
    double result = value * (head_p_value / max_p_value);
    if (_isnan(result)) result = 0;
    return result;

    if (0) {
      // �� 2���񶯿�ʼ���
      double max_value = max_p_value;
      double last_value = max_value;
      int monotonicity = 0;
      int v_n;
      for (v_n = 4; v_n < vib_n; v_n *= 2) {
        double value = dct_func(s, (int)(v_n * n1), n_end, v_n, n1);

        if (value > max_value)
          max_value = value;

        double sc = value / last_value;
        if (sc > 0) {
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
      //sample_pos = (int)((double)milliseconds / 1000 * m_pcm_->m_num_samples_per_sec_);

      // ȷ���񶯴�����ȷ�������������õ���Сʱ��ֱ���
      //vib_n = freq / MAX_TIME_RAT;

      // ȷ���񶯴��������������õ�����񶯴���
      //if (vib_n > MAX_VIB_N) vib_n = MAX_VIB_N;

      //if (vib_n < 1) vib_n = 1;

      //return sample_pos_dct(sample_pos, channel, freq, vib_n);

      if (head_p_value > max_p_value) head_p_value = max_p_value;
      double result = last_value * (head_p_value / max_p_value);
      if (_isnan(result)) result = 0;
      if (result > m_pcm_->m_max_amplitude_) result = m_pcm_->m_max_amplitude_;
      //result = sample_pos_dct(sample_pos, channel, freq, vib_n) / 8;
      return result;
    }
  }

  inline double GetAmpOfFreq(const int milliseconds, const double freq) {
    double sum = 0;
    for (int i = 0; i < m_pcm_->m_num_channels_; i++)
      sum += GetAmpOfFreq(milliseconds, i, freq);
    return sum / m_pcm_->m_num_channels_;
  }

 private:
  CPCMStream<TYPE> *m_pcm_;

  double *cos_div_, *sin_div_;

  // ���������Ż��������������
  TYPE *m_td_buf_;
  TYPE *m_td_[MAX_CHANNELS][32];
};


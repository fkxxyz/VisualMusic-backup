
// audio_pcm_analyser.h

#pragma once

#include <cfloat>

// 最小振动次数（不包含低频检测）
#define MIN_VIB_N  32.0

// 最大振动次数
#define MAX_VIB_N  128.0

// 最小时间分辨率（至少把一秒钟分成多少份）（达到最小分辨率时自动降低振动次数）
#define MIN_TIME_RAT  1
#define MAX_TIME_RAT  50

// 坍塌比例值
#define MD_SCALE_UPPER  65536.0l

// 振动一次最多需要的样点数量，此值越大，优化合并的点越小，效率更低
#define MAX_N1  8

// 此值限制最大 DCT 计算数量
#define MAX_N  512

// 提前计算sin和cos的值提升效率，此值表示将一个圆分成多少份计算
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

    // 设置固定参数
    m_pcm_ = pcm;

    // 计算 cos 、 sin 的分割优化数组
    cos_div_ = new double [SIN_COS_RATE * 2];
    sin_div_ = cos_div_ + SIN_COS_RATE;
    for (int i = 0; i < SIN_COS_RATE; i++) {
      double angle = (2 * M_PI) * ((double)i / SIN_COS_RATE);
      cos_div_[i] = cos(angle);
      sin_div_[i] = sin(angle);
    }

    // 计算样本的缩放次数
    int max_n = pcm->m_num_samples_;
    int max_optimizeR = 0;
    while (max_n > MAX_N) {
      max_n >>= 1;
      max_optimizeR++;
    }

    // 缩放样本
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
    // 计算要用 DCT 公式涵盖的样点数
    double n1 = m_pcm_->m_num_samples_per_sec_ / freq;
    int n = (int)(vib_n * n1);

    // 确保不超过歌曲结尾
    int n_end = n;
    if (sample_pos + n > m_pcm_->m_num_samples_)
      n_end = m_pcm_->m_num_samples_ - sample_pos;
    if (n_end < 0) return 0l;

    int r = 0;
    int s_pos = sample_pos;
    // 计算优化点数并优化
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

    // 计算此时间对应的样点位置
    int sample_pos = (int)(
        (double)milliseconds / 1000 * m_pcm_->m_num_samples_per_sec_);

    // 确定振动次数，确保不超过所设置的最小时间分辨率
    double vib_n = freq / MAX_TIME_RAT;

    // 确保振动次数不超过所设置的最大振动次数
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

    // 振动一次所需样点数

    // 最大分辨率内最多振动的次数
    int time_pix_vib_n = (int)(freq / MAX_TIME_RAT);
    if (time_pix_vib_n == 0) time_pix_vib_n = 1;

    // 计算要用 DCT 公式涵盖的样点数
    double n1 = m_pcm_->m_num_samples_per_sec_ / freq;
    int N = (int)(vib_n * n1);

    // 确保不超过歌曲结尾
    int n_end = N;
    if (sample_pos + N > m_pcm_->m_num_samples_)
      n_end = m_pcm_->m_num_samples_ - sample_pos;
    if (n_end < 0) return 0l;

    int r = 0;
    // 计算优化点数并优化
    while (n1 > MAX_N1) {
      n1 /= 2;
      N >>= 1;
      n_end >>= 1;
      sample_pos >>= 1;
      r++;
    }

    TYPE *s = m_td_[channel][r] + sample_pos;

    // 游离检测
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
      // 从 2次振动开始检测
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

      // 计算此时间对应的样点位置
      //sample_pos = (int)((double)milliseconds / 1000 * m_pcm_->m_num_samples_per_sec_);

      // 确定振动次数，确保不超过所设置的最小时间分辨率
      //vib_n = freq / MAX_TIME_RAT;

      // 确保振动次数不超过所设置的最大振动次数
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

  // 保存用于优化的样本相关数据
  TYPE *m_td_buf_;
  TYPE *m_td_[MAX_CHANNELS][32];
};


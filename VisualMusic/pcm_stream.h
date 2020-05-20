
// pcm_stream.h
#pragma once


#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>

#define MAX_CHANNELS  32

template <class TYPE>
class CPCMStream {
 public:
  int m_num_samples_per_sec_;
  int m_num_channels_;
  int m_num_samples_;
  TYPE *m_samples_[MAX_CHANNELS];

  TYPE m_max_amplitude_;
  TYPE m_origin_pos_;

  CPCMStream():m_num_channels_(0) {}
  CPCMStream(
      TYPE (*func)(double),
      const int milliseconds = 1000,
      const double freq = 440,
      const int num_samples_per_sec = 44100,
      const TYPE type_max = 0,
      const TYPE type_min = 0)
      : m_num_channels_(0) {
    Generate(func, milliseconds, freq, num_samples_per_sec, type_max, type_min);
  }
  CPCMStream(TYPE **samples, const int num_channels, const int num_samples,
             const int num_samples_per_sec, const int origin_pos,
             const int max_amplitude)
    : m_num_channels_(num_channels),
      m_num_samples_(num_samples),
      m_num_samples_per_sec_(num_samples_per_sec),
      m_origin_pos_(origin_pos),
      m_max_amplitude_(max_amplitude)  {
    assert(samples);
    assert(num_channels > 0);
    assert(num_samples > 0);
    assert(num_samples_per_sec > 0);

    for (int i = 0; i < m_num_channels_; i++) {
      m_samples_[i] = new TYPE [m_num_samples_];
      for (int j = 0; j < m_num_samples_; j++)
        m_samples_[i][j] = samples[i][j];
    }
  }
  ~CPCMStream() { Clear(); }

  void Generate(
      const TYPE (*func)(double),
      const int milliseconds = 1000,
      const double freq = 440,
      const int num_samples_per_sec = 44100,
      const TYPE type_max = 0,
      const TYPE type_min = 0) {
    assert(func);
    assert(milliseconds > 0);
    assert(num_samples_per_sec > 0);

    Clear();

    m_num_samples_per_sec_ = num_samples_per_sec;
    m_num_channels_ = 1;
    m_num_samples_ = (int)((double)milliseconds * num_samples_per_sec / 1000);

    *m_samples_ = new TYPE [m_num_samples_];

    TYPE max = type_max, min = type_min;
    for (int i = 0; i < m_num_samples_; i++) {
      double x = (double)i * (2.0l * M_PI) * freq / m_num_samples_per_sec_;
      TYPE r = func(x);
      if (r > max) max = r;
      if (r < min) min = r;
      (*m_samples_)[i] = r;
    }

    m_max_amplitude_ = max / 3 * 2 - min / 3 * 2;
    m_origin_pos_ = min + m_max_amplitude_;
  }

  void Clear() {
    if (m_num_channels_ > 0)
      for (int i = 0; i < m_num_channels_; i++)
        delete m_samples_[i];
    m_num_channels_ = 0;
  }
};


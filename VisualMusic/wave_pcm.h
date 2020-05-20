
// wave_pcm.h

#pragma once

#include <cassert>
#include "win_wave_file.h"
#include "win_pcm_player.h"
#include "pcm_stream.h"

class CWavePCM {
 public:
  WAVEFORMATEX m_format_;
  char *m_data_;
  LONG m_size_;

  CWavePCM():m_data_(NULL) {}
  CWavePCM(LPCTSTR sz_file_name):m_data_(NULL) {
    if (!Load(sz_file_name))
      assert(0);
  }
  CWavePCM(
      CPCMStream<double> *pcm,
      const int index_channel,
      const int bits_per_sample)
      : m_data_(NULL) {
    Load(pcm, index_channel, bits_per_sample);
  }
  CWavePCM(CPCMStream<double> *pcm, const int bits_per_sample = 16)
      : m_data_(NULL) {
    Load(pcm, bits_per_sample);
  }
  ~CWavePCM() {Clear();}

  void Clear() {
    if (m_data_) delete m_data_;
    m_data_ = NULL;
  }

  inline BOOL Load(LPCTSTR sz_file_name) {
    Clear();

    // 打开
    CWaveFile file;
    if (!file.Open(sz_file_name))
      return FALSE;

    // 读取
    m_format_ = *file.GetFormatExInfo();
    m_format_.cbSize = 0;
    m_size_ = file.GetDataSize();
    m_data_ = new char [m_size_];
    if (!file.Read(m_data_, m_size_))
      return FALSE;

    // 关闭
    if (file.Close())
      assert(0);

    return TRUE;
  }

  inline void Load(CPCMStream<double> *pcm,
                   const int index_channel, const int bits_per_sample) {
    Clear();

    m_format_.wFormatTag = bits_per_sample == 32 ? 3 : WAVE_FORMAT_PCM;
    m_format_.nSamplesPerSec = 44100;
    m_format_.nChannels = 1;
    m_format_.wBitsPerSample = bits_per_sample;
    m_format_.nBlockAlign =
        m_format_.nChannels * (m_format_.wBitsPerSample / 8);
    m_format_.nAvgBytesPerSec =
        m_format_.nSamplesPerSec * m_format_.nBlockAlign;
    m_format_.cbSize = 0;

    m_size_ = pcm->m_num_samples_ * (m_format_.wBitsPerSample / 8);
    m_data_ = new char [m_size_];

    double scale, d;
    switch (bits_per_sample) {
    case 8:
      scale = 128.0l / pcm->m_max_amplitude_;
      d = 128.0l - pcm->m_origin_pos_;
      for (int i = 0; i < pcm->m_num_samples_; i++)
        ((unsigned char *)m_data_)[i] =
            (unsigned char)(pcm->m_samples_[index_channel][i] * scale + d);
      break;
    case 16:
      scale = 32768.0l / pcm->m_max_amplitude_;
      d = - pcm->m_origin_pos_;
      for (int i = 0; i < pcm->m_num_samples_; i++)
        ((short *)m_data_)[i] =
            (short)(pcm->m_samples_[index_channel][i] * scale + d);
      break;
    case 32:
      scale = 1.0l / pcm->m_max_amplitude_;
      d = - pcm->m_origin_pos_;
      for (int i = 0; i < pcm->m_num_samples_; i++)
        ((float *)m_data_)[i] =
            (float)(pcm->m_samples_[index_channel][i] * scale + d);
      break;
    }
  }

  inline void Load(CPCMStream<double> *pcm, const int bits_per_sample = 16) {
    Clear();

    m_format_.wFormatTag = bits_per_sample == 32 ? 3 : WAVE_FORMAT_PCM;
    m_format_.nSamplesPerSec = pcm->m_num_samples_per_sec_;
    m_format_.nChannels = pcm->m_num_channels_;
    m_format_.wBitsPerSample = bits_per_sample;
    m_format_.nBlockAlign =
        m_format_.nChannels * (m_format_.wBitsPerSample / 8);
    m_format_.nAvgBytesPerSec =
        m_format_.nSamplesPerSec * m_format_.nBlockAlign;
    m_format_.cbSize = 0;

    m_size_ =
        pcm->m_num_samples_ *
        (m_format_.wBitsPerSample / 8) *
        m_format_.nChannels;
    m_data_ = new char [m_size_];

    double scale, d;
    switch (bits_per_sample) {
    case 8:
      scale = 127.0l / pcm->m_max_amplitude_;
      d = 128.0l - pcm->m_origin_pos_;
      {
        unsigned char *p = (unsigned char *)m_data_;
        for (int i = 0; i < pcm->m_num_samples_; i++)
          for (int n = 0; n < m_format_.nChannels; n++) {
#ifdef _DEBUG
            int pp = (int)(pcm->m_samples_[n][i] * scale + d);
            assert(pp == (int)(unsigned char)pp);
#endif
            *(p++) = (unsigned char)(pcm->m_samples_[n][i] * scale + d);
          }
      }
      break;
    case 16:
      scale = 32767.0l / pcm->m_max_amplitude_;
      d = - pcm->m_origin_pos_;
      {
        short *p = (short *)m_data_;
        for (int i = 0; i < pcm->m_num_samples_; i++)
          for (int n = 0; n < m_format_.nChannels; n++) {
#ifdef _DEBUG
            int pp = (int)(pcm->m_samples_[n][i] * scale + d);
            assert(pp == (int)(short)pp);
#endif
            *(p++) = (short)(pcm->m_samples_[n][i] * scale + d);
          }
      }
      break;
    case 32:
      scale = 1.0l / pcm->m_max_amplitude_;
      d = - pcm->m_origin_pos_;
      {
        float *p = (float *)m_data_;
        for (int i = 0; i < pcm->m_num_samples_; i++)
          for (int n = 0; n < m_format_.nChannels; n++) {
            *(p++) = (float)(pcm->m_samples_[n][i] * scale + d);
          }
      }
      break;
    }
  }

  inline BOOL Save(LPCTSTR sz_file_name) {
    assert(m_data_);

    // 打开
    CWaveFile file;
    if (!file.Open(sz_file_name, MMIO_WRITE | MMIO_CREATE))
      return FALSE;

    // 写入
    if (!file.WriteFormatExInfo(&m_format_))
      return FALSE;

    if (file.Write(m_data_, m_size_) != m_size_)
      return FALSE;

    // 关闭
    if (file.Close())
      assert(0);

    return TRUE;
  }
};


class CWavePCMPlayer : public CWavePCM, public CPCMPlayer<2>{

 public:
  CWavePCMPlayer() {}
  CWavePCMPlayer(LPCTSTR sz_file_name):CWavePCM(sz_file_name) {}
  CWavePCMPlayer(CPCMStream<double> *pcm, const int index_channel,
                 const int bits_per_sample)
      : CWavePCM(pcm, index_channel, bits_per_sample) {}
  CWavePCMPlayer(CPCMStream<double> *pcm, const int bits_per_sample = 16)
      : CWavePCM(pcm, bits_per_sample) {}

  BOOL Open() { return CPCMPlayer<2>::Open(&m_format_);}
  BOOL Play() {
    if (!CPCMPlayer<2>::Reset())
      return 0;
    return CPCMPlayer<2>::Insert(m_data_, m_size_);
  }

  void Clear() {
    CPCMPlayer<2>::Clear();
    CWavePCM::Clear();
  }

  BOOL PlayAndWait() {
    if (!Open())
      return FALSE;

    if (!Play())
      return FALSE;

    if (!CPCMPlayer<2>::Wait())
      return FALSE;

    if (!CPCMPlayer<2>::Close())
      return FALSE;

    return TRUE;
  }
};


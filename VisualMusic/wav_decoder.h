
// wav_decoder.h

#pragma once 
#include "audio_decoder_template.h"
#include "wave_pcm.h"

class CWavDecoder : public CAudioDecoderTemplate<const TCHAR> {
 public:
  int IsValidFormat(LPCTSTR filepath) {
    assert(0);
    return 0;
  }

  int Decode(LPCTSTR filepath, CPCMStream<double> *pcm) {
    CWavePCM wave_pcm;
    if (!wave_pcm.Load(filepath))
      return 0;

    pcm->Clear();
    pcm->m_num_samples_per_sec_ = wave_pcm.m_format_.nSamplesPerSec;
    pcm->m_num_channels_ = wave_pcm.m_format_.nChannels;
    pcm->m_num_samples_ = wave_pcm.m_size_ / wave_pcm.m_format_.nBlockAlign;
    switch (wave_pcm.m_format_.wBitsPerSample) {
    case 8:
      pcm->m_max_amplitude_ = 128.0l;
      pcm->m_origin_pos_ = 128.0l;
      for (int i = 0; i < pcm->m_num_channels_; i++) {
        pcm->m_samples_[i] = new double [pcm->m_num_samples_];
        for (register int j = 0; j < pcm->m_num_samples_; j++)
          pcm->m_samples_[i][j] = 
              *((unsigned char *)wave_pcm.m_data_ +
                pcm->m_num_channels_ * j + i);
      }
      break;
    case 16:
      pcm->m_max_amplitude_ = 32768.0l;
      pcm->m_origin_pos_ = 0.0l;
      for (int i = 0; i < pcm->m_num_channels_; i++) {
        pcm->m_samples_[i] = new double [pcm->m_num_samples_];
        for (register int j = 0; j < pcm->m_num_samples_; j++)
          pcm->m_samples_[i][j] =
              *((short *)wave_pcm.m_data_ +
                pcm->m_num_channels_ * j + i);
      }
      break;
    case 32:
      pcm->m_max_amplitude_ = 1.0l;
      pcm->m_origin_pos_ = 0.0l;
      for (int i = 0; i < pcm->m_num_channels_; i++) {
        pcm->m_samples_[i] = new double [pcm->m_num_samples_];
        for (register int j = 0; j < pcm->m_num_samples_; j++) {
          register double x =
              *((float *)wave_pcm.m_data_ +
                pcm->m_num_channels_ * j + i);
          if (x > 1.0l) x = 1.0l;
          if (x < -1.0l) x = -1.0l;
          pcm->m_samples_[i][j] = x;
        }
      }
      break;
    }

    return 1;
  }
};

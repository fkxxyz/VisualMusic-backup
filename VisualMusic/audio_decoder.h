
// audio_decoder.h
#pragma once

#include "wav_decoder.h"
#include "mp3_decoder.h"

#define COUNT_OF_DECODER  2

class CAudioDecoder {
 public:
  CAudioDecoder() {
    m_decoder_[0] = &m_d_wav_;
    m_decoder_[1] = &m_d_mp3_;
    m_decoder_[2] = NULL;
  }

  BOOL Decode(LPCTSTR filepath, CPCMStream<double> *pcm) {
    for (CAudioDecoderTemplate<const TCHAR> **pd = m_decoder_; *pd; pd++) {
      if ((*pd)->Decode(filepath, pcm))
        return TRUE;
    }
    return FALSE;
  }

 private:
  CAudioDecoderTemplate<const TCHAR> *m_decoder_[COUNT_OF_DECODER + 1];

  CWavDecoder m_d_wav_;
  CMp3Decoder<const TCHAR> m_d_mp3_;

};



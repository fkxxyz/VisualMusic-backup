
// audio_decoder_template.h

#pragma once
#include "pcm_stream.h"
#include "wave_pcm.h"

template <class CHARTYPE>
class CAudioDecoderTemplate {
 public:
  virtual int IsValidFormat(CHARTYPE *filepath) = 0;
  virtual int Decode(CHARTYPE *filepath, CPCMStream<double> *pcm) = 0;
};



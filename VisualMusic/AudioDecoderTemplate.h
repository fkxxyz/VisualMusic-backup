
// AudioDecoderTemplate.h

#pragma once
#include "PCMStream.h"
#include "WavePCM.h"

template <class CHARTYPE>
class CAudioDecoderTemplate {
public:
	virtual int IsValidFormat(CHARTYPE *filepath) = 0;
	virtual int Decode(CHARTYPE *filepath, CPCMStream<double> *pcm) = 0;
};




// AudioDecoder.h
#pragma once

#include "WavDecoder.h"
#include "Mp3Decoder.h"

#define COUNT_OF_DECODER	2

class CAudioDecoder {
private:
	CAudioDecoderTemplate<TCHAR> *m_Decoder[COUNT_OF_DECODER + 1];

	CWavDecoder m_d_wav;
	CMp3Decoder<TCHAR> m_d_mp3;

public:
	CAudioDecoder(){
		m_Decoder[0] = &m_d_wav;
		m_Decoder[1] = &m_d_mp3;
		m_Decoder[2] = NULL;
	}

	BOOL Decode(LPTSTR filepath, CPCMStream<double> *pcm){
		for (CAudioDecoderTemplate<TCHAR> **pd = m_Decoder; *pd; pd++){
			if ((*pd)->Decode(filepath, pcm))
				return TRUE;
		}
		return FALSE;
	}
};




// WavDecoder.h

#pragma once 
#include "AudioDecoderTemplate.h"
#include "WavePCM.h"

class CWavDecoder : public CAudioDecoderTemplate<TCHAR> {
public:
	int IsValidFormat(LPTSTR filepath){
		assert(0);
		return 0;
	}

	int Decode(LPTSTR filepath, CPCMStream<double> *pcm){
		CWavePCM WavePcm;
		if (!WavePcm.Load(filepath))
			return 0;

		pcm->Clear();
		pcm->m_nSamplesPerSec = WavePcm.m_Format.nSamplesPerSec;
		pcm->m_nChannels = WavePcm.m_Format.nChannels;
		pcm->m_nSamples = WavePcm.m_Size / WavePcm.m_Format.nBlockAlign;
		switch (WavePcm.m_Format.wBitsPerSample){
		case 8:
			pcm->m_max_amplitude = 128.0l;
			pcm->m_origin_pos = 128.0l;
			for (int i = 0; i < pcm->m_nChannels; i++){
				pcm->m_Samples[i] = new double [pcm->m_nSamples];
				for (register int j = 0; j < pcm->m_nSamples; j++)
					pcm->m_Samples[i][j] = *((unsigned char *)WavePcm.m_Data + pcm->m_nChannels * j + i);
			}
			break;
		case 16:
			pcm->m_max_amplitude = 32768.0l;
			pcm->m_origin_pos = 0.0l;
			for (int i = 0; i < pcm->m_nChannels; i++){
				pcm->m_Samples[i] = new double [pcm->m_nSamples];
				for (register int j = 0; j < pcm->m_nSamples; j++)
					pcm->m_Samples[i][j] = *((short *)WavePcm.m_Data + pcm->m_nChannels * j + i);
			}
			break;
		case 32:
			pcm->m_max_amplitude = 1.0l;
			pcm->m_origin_pos = 0.0l;
			for (int i = 0; i < pcm->m_nChannels; i++){
				pcm->m_Samples[i] = new double [pcm->m_nSamples];
				for (register int j = 0; j < pcm->m_nSamples; j++){
					register double x =  *((float *)WavePcm.m_Data + pcm->m_nChannels * j + i);
					if (x > 1.0l) x = 1.0l;
					if (x < -1.0l) x = -1.0l;
					pcm->m_Samples[i][j] = x;
				}
			}
			break;
		}

		return 1;
	}
};

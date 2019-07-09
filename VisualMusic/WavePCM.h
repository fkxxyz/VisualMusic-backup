
// WavePCM.h

#pragma once

#include <cassert>
#include "WinWaveFile.h"
#include "WinPCMPlayer.h"
#include "PCMStream.h"

class CWavePCM {
public:
	WAVEFORMATEX m_Format;
	char *m_Data;
	LONG m_Size;

	CWavePCM():m_Data(NULL){}
	CWavePCM(LPTSTR szFileName):m_Data(NULL){
		if (!Load(szFileName))
			assert(0);
	}
	CWavePCM(CPCMStream<double> *pcm, int indexChannel, int BitsPerSample):m_Data(NULL){
		Load(pcm, indexChannel, BitsPerSample);
	}
	CWavePCM(CPCMStream<double> *pcm, int BitsPerSample = 16):m_Data(NULL){
		Load(pcm, BitsPerSample);
	}
	~CWavePCM(){Clear();}

	void Clear(){
		if (m_Data) delete m_Data;
		m_Data = NULL;
	}

	inline BOOL Load(LPTSTR szFileName){
		Clear();

		// 打开
		CWaveFile file;
		if (!file.Open(szFileName))
			return FALSE;

		// 读取
		m_Format = *file.GetFormatExInfo();
		m_Format.cbSize = 0;
		m_Size = file.GetDataSize();
		m_Data = new char [m_Size];
		if (!file.Read(m_Data, m_Size))
			return FALSE;

		// 关闭
		if (file.Close())
			assert(0);

		return TRUE;
	}

	inline void Load(CPCMStream<double> *pcm, int indexChannel, int BitsPerSample){
		Clear();

		m_Format.wFormatTag = BitsPerSample == 32 ? 3 : WAVE_FORMAT_PCM;
		m_Format.nSamplesPerSec = 44100;
		m_Format.nChannels = 1;
		m_Format.wBitsPerSample = BitsPerSample;
		m_Format.nBlockAlign = m_Format.nChannels * (m_Format.wBitsPerSample / 8);
		m_Format.nAvgBytesPerSec = m_Format.nSamplesPerSec * m_Format.nBlockAlign;
		m_Format.cbSize = 0;

		m_Size = pcm->m_nSamples * (m_Format.wBitsPerSample / 8);
		m_Data = new char [m_Size];

		double scale, d;
		switch (BitsPerSample){
		case 8:
			scale = 128.0l / pcm->m_max_amplitude;
			d = 128.0l - pcm->m_origin_pos;
			for (int i = 0; i < pcm->m_nSamples; i++)
				((unsigned char *)m_Data)[i] = (unsigned char)(pcm->m_Samples[indexChannel][i] * scale + d);
			break;
		case 16:
			scale = 32768.0l / pcm->m_max_amplitude;
			d = - pcm->m_origin_pos;
			for (int i = 0; i < pcm->m_nSamples; i++)
				((short *)m_Data)[i] = (short)(pcm->m_Samples[indexChannel][i] * scale + d);
			break;
		case 32:
			scale = 1.0l / pcm->m_max_amplitude;
			d = - pcm->m_origin_pos;
			for (int i = 0; i < pcm->m_nSamples; i++)
				((float *)m_Data)[i] = (float)(pcm->m_Samples[indexChannel][i] * scale + d);
			break;
		}
	}

	inline void Load(CPCMStream<double> *pcm, int BitsPerSample = 16){
		Clear();

		m_Format.wFormatTag = BitsPerSample == 32 ? 3 : WAVE_FORMAT_PCM;
		m_Format.nSamplesPerSec = pcm->m_nSamplesPerSec;
		m_Format.nChannels = pcm->m_nChannels;
		m_Format.wBitsPerSample = BitsPerSample;
		m_Format.nBlockAlign = m_Format.nChannels * (m_Format.wBitsPerSample / 8);
		m_Format.nAvgBytesPerSec = m_Format.nSamplesPerSec * m_Format.nBlockAlign;
		m_Format.cbSize = 0;

		m_Size = pcm->m_nSamples * (m_Format.wBitsPerSample / 8) * m_Format.nChannels;
		m_Data = new char [m_Size];

		double scale, d;
		switch (BitsPerSample){
		case 8:
			scale = 127.0l / pcm->m_max_amplitude;
			d = 128.0l - pcm->m_origin_pos;
			{
				unsigned char *p = (unsigned char *)m_Data;
				for (int i = 0; i < pcm->m_nSamples; i++)
					for (int n = 0; n < m_Format.nChannels; n++){
#ifdef _DEBUG
						int pp = (int)(pcm->m_Samples[n][i] * scale + d);
						assert(pp == (int)(unsigned char)pp);
#endif
						*(p++) = (unsigned char)(pcm->m_Samples[n][i] * scale + d);
					}
			}
			break;
		case 16:
			scale = 32767.0l / pcm->m_max_amplitude;
			d = - pcm->m_origin_pos;
			{
				short *p = (short *)m_Data;
				for (int i = 0; i < pcm->m_nSamples; i++)
					for (int n = 0; n < m_Format.nChannels; n++){
#ifdef _DEBUG
						int pp = (int)(pcm->m_Samples[n][i] * scale + d);
						assert(pp == (int)(short)pp);
#endif
						*(p++) = (short)(pcm->m_Samples[n][i] * scale + d);
					}
			}
			break;
		case 32:
			scale = 1.0l / pcm->m_max_amplitude;
			d = - pcm->m_origin_pos;
			{
				float *p = (float *)m_Data;
				for (int i = 0; i < pcm->m_nSamples; i++)
					for (int n = 0; n < m_Format.nChannels; n++){
						*(p++) = (float)(pcm->m_Samples[n][i] * scale + d);
					}
			}
			break;
		}
	}

	inline BOOL Save(LPTSTR szFileName){
		assert(m_Data);

		// 打开
		CWaveFile file;
		if (!file.Open(szFileName, MMIO_WRITE | MMIO_CREATE))
			return FALSE;

		// 写入
		if (!file.WriteFormatExInfo(&m_Format))
			return FALSE;

		if (file.Write(m_Data, m_Size) != m_Size)
			return FALSE;

		// 关闭
		if (file.Close())
			assert(0);

		return TRUE;
	}
};


class CWavePCMPlayer : public CWavePCM, public CPCMPlayer<2>{

public:
	CWavePCMPlayer(){}
	CWavePCMPlayer(LPTSTR szFileName):CWavePCM(szFileName){}
	CWavePCMPlayer(CPCMStream<double> *pcm, int indexChannel, int BitsPerSample):CWavePCM(pcm, indexChannel, BitsPerSample){}
	CWavePCMPlayer(CPCMStream<double> *pcm, int BitsPerSample = 16):CWavePCM(pcm, BitsPerSample){}

	BOOL Open(){ return CPCMPlayer<2>::Open(&m_Format);}
	BOOL Play(){
		if (!CPCMPlayer<2>::Reset())
			return 0;
		return CPCMPlayer<2>::Insert(m_Data, m_Size);
	}

	void Clear(){
		CPCMPlayer<2>::Clear();
		CWavePCM::Clear();
	}

	BOOL PlayAndWait(){
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


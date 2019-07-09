
// PCMStream.h
#pragma once


#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>

#define MAX_CHANNELS	32

template <class TYPE>
class CPCMStream {
public:
	int m_nSamplesPerSec;
	int m_nChannels;
	int m_nSamples;
	TYPE *m_Samples[MAX_CHANNELS];

	TYPE m_max_amplitude;
	TYPE m_origin_pos;

	CPCMStream():m_nChannels(0){}
	CPCMStream(TYPE (*func)(double), int nMilliseconds = 1000, double freq = 440, int nSamplesPerSec = 44100, TYPE type_max = 0, TYPE type_min = 0):m_nChannels(0){
		Generate(func, nMilliseconds, freq, nSamplesPerSec, type_max, type_min);
	}
	CPCMStream(TYPE **Samples, int nChannels, int nSamples, int nSamplesPerSec, int origin_pos, int max_amplitude):
		m_nChannels(nChannels),m_nSamples(nSamples),m_nSamplesPerSec(nSamplesPerSec), m_origin_pos(origin_pos), m_max_amplitude(max_amplitude)
	{
		assert(Samples);
		assert(nChannels > 0);
		assert(nSamples > 0);
		assert(nSamplesPerSec > 0);

		for (int i = 0; i < m_nChannels; i++){
			m_Samples[i] = new TYPE [m_nSamples];
			for (int j = 0; j < m_nSamples; j++)
				m_Samples[i][j] = Samples[i][j];
		}
	}
	~CPCMStream(){ Clear(); }

	void Generate(TYPE (*func)(double), int nMilliseconds = 1000, double freq = 440, int nSamplesPerSec = 44100, TYPE type_max = 0, TYPE type_min = 0){
		assert(func);
		assert(nMilliseconds > 0);
		assert(nSamplesPerSec > 0);

		Clear();

		m_nSamplesPerSec = nSamplesPerSec;
		m_nChannels = 1;
		m_nSamples = (int)((double)nMilliseconds * nSamplesPerSec / 1000);

		*m_Samples = new TYPE [m_nSamples];

		TYPE max = type_max, min = type_min;
		for (int i = 0; i < m_nSamples; i++){
			double x = (double)i * (2.0l * M_PI) * freq / m_nSamplesPerSec;
			TYPE r = func(x);
			if (r > max) max = r;
			if (r < min) min = r;
			(*m_Samples)[i] = r;
		}

		m_max_amplitude = max / 3 * 2 - min / 3 * 2;
		m_origin_pos = min + m_max_amplitude;
	}

	void Clear(){
		if (m_nChannels > 0)
			for (int i = 0; i < m_nChannels; i++)
				delete m_Samples[i];
		m_nChannels = 0;
	}
};



// WinWaveFile.h

#pragma once
#include "WinMMwaveOut.h"

#define FOURCC_WAVE     mmioFOURCC('W', 'A', 'V', 'E')
#define FOURCC_fmt      mmioFOURCC('f', 'm', 't', ' ')
#define FOURCC_data      mmioFOURCC('d', 'a', 't', 'a')

class CWaveFile : protected CWinMMIO {

protected:
	PWAVEFORMATEX m_pWaveFormatEx;
	WAVEFORMATEX m_WaveFormatEx;

	DWORD m_dwOpenFlags;

	MMCKINFO m_ckData;

public:
	CWaveFile::CWaveFile():m_pWaveFormatEx(NULL){}

	BOOL Open(LPTSTR szFileName, DWORD dwOpenFlags = MMIO_READ);

	inline MMRESULT Close(){
		assert(m_hIO != NULL);

		// 如果是写 wav ，则关闭前要在 riff 块和 data 块中加上大小信息
		if (m_dwOpenFlags & MMIO_WRITE){
			assert(m_pWaveFormatEx != NULL);

			// 得出整个文件的大小
			LONG size = CWinMMIO::Seek(0, SEEK_END);

			// 得出并写入 riff 块大小
			DWORD dwRiffSize = size - 8;
			if (CWinMMIO::Seek(4) != 4)
				assert(0);
			if (CWinMMIO::Write((char *)&dwRiffSize, 4) != 4)
				assert(0);

			// 得出并写入 data 块大小
			DWORD dwDataSize = size - m_ckData.dwDataOffset;
			if (CWinMMIO::Seek(m_ckData.dwDataOffset - 4) != m_ckData.dwDataOffset - 4)
				assert(0);
			if (CWinMMIO::Write((char *)&dwDataSize, 4) != 4)
				assert(0);
		}

		if (m_pWaveFormatEx && m_pWaveFormatEx != &m_WaveFormatEx)
			delete m_pWaveFormatEx;
		m_pWaveFormatEx = NULL;

		return CWinMMIO::Close();
	}

	inline PWAVEFORMATEX GetFormatExInfo() const{
		assert(m_hIO != NULL);

		return m_pWaveFormatEx;
	}

	BOOL WriteFormatExInfo(PWAVEFORMATEX pWaveFormatEx);

	inline LONG Write( const char * pch, LONG cch){
		assert(m_hIO != NULL);
		assert(m_dwOpenFlags & MMIO_WRITE);
		assert(m_pWaveFormatEx != NULL);

		return CWinMMIO::Write(pch, cch);
	}

	inline DWORD GetDataSize() const{
		assert(m_hIO != NULL);
		assert((m_dwOpenFlags & MMIO_WRITE) == 0);

		return m_ckData.cksize;
	}

	inline BOOL DataSeek( LONG lDataOffset){
		assert(m_hIO != NULL);
		assert(m_pWaveFormatEx != NULL);

		LONG lOffset = lDataOffset + m_ckData.dwDataOffset;
		return CWinMMIO::Seek(lOffset) == lOffset;
	}

	inline LONG Read( OUT HPSTR pch, IN LONG cch){
		assert(m_hIO != NULL);
		assert((m_dwOpenFlags & MMIO_WRITE) == 0);

		return CWinMMIO::Read(pch, cch);
	}
};


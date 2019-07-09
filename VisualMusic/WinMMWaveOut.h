
// WinMMwaveOut.h

#pragma once

#include <windows.h>
#include <cassert>
#include <mmsystem.h>

typedef void (CALLBACK *WAVEOUTCALLBACKPROC)(
   HWAVEOUT  hwo,
   UINT      uMsg,
   DWORD_PTR dwInstance,
   DWORD_PTR dwParam1,
   DWORD_PTR dwParam2
);

class CWinMMWaveOut {

private:
	HWAVEOUT m_hWavaOut;
	MMRESULT mmrLastResult;

public:
	CWinMMWaveOut():m_hWavaOut(NULL){}
	~CWinMMWaveOut(){if (m_hWavaOut) Close(); assert(m_hWavaOut==NULL);}

	explicit CWinMMWaveOut(HWAVEOUT handle):m_hWavaOut(NULL){ Attach(handle);}
	explicit CWinMMWaveOut(const CWinMMWaveOut &object):m_hWavaOut(NULL){ Attach(object.m_hWavaOut); }

	void Attach(HWAVEOUT handle){ assert(m_hWavaOut==NULL); m_hWavaOut=handle; }
	void Detach(){ assert(m_hWavaOut!=NULL); m_hWavaOut=NULL; }

	static UINT GetNumDevs();
	static MMRESULT GetDevCaps(UINT_PTR uDeviceID, LPWAVEOUTCAPS pwoc, UINT cbwoc);

	BOOL GetVolume(LPDWORD pdwVolume);
	BOOL GetVolume(DWORD dwVolume);

	BOOL Open(LPCWAVEFORMATEX pwfx, UINT uDeviceID);
	BOOL Open(LPCWAVEFORMATEX pwfx, WAVEOUTCALLBACKPROC waveOutProc, DWORD_PTR dwInstance, UINT uDeviceID);
	BOOL Open(LPCWAVEFORMATEX pwfx, HWND hWindow, UINT uDeviceID);
	BOOL Open(LPCWAVEFORMATEX pwfx, DWORD dwThreadID, DWORD_PTR dwInstance, UINT uDeviceID);
	BOOL Open(LPCWAVEFORMATEX pwfx, HANDLE hEvent, DWORD_PTR dwInstance, UINT uDeviceID);
	BOOL Close();

	BOOL PrepareHeader(LPWAVEHDR pwh, UINT cbwh);
	BOOL UnprepareHeader(LPWAVEHDR pwh, UINT cbwh);
	BOOL Write(LPWAVEHDR pwh, UINT cbwh);
	BOOL Pause(); // 暂停
	BOOL Restart(); // 继续
	BOOL Reset();  // 停止（相当于直接跳到结尾）
	BOOL BreakLoop();
	BOOL GetPosition(LPMMTIME pmmt, UINT cbmmt);
	BOOL GetPitch(LPDWORD pdwPitch);
	BOOL SetPitch(DWORD dwPitch);
	BOOL GetPlaybackRate(LPDWORD pdwRate);
	BOOL SetPlaybackRate(DWORD dwRate);
	BOOL GetID(LPUINT puDeviceID);

	MMRESULT GetLastResult();
	static BOOL GetErrorText(MMRESULT mmrError, LPTSTR pszText, UINT cchText);

};

#include "WinMMwaveOut.inl"



class CWinMMIO {

protected:
	HMMIO m_hIO;
	static const DWORD LastErrorTab[];

public:
	CWinMMIO():m_hIO(NULL){}
	~CWinMMIO(){if (m_hIO) Close(); assert(m_hIO==NULL);}

	explicit CWinMMIO(HMMIO handle):m_hIO(NULL){ Attach(handle);}
	explicit CWinMMIO(const CWinMMIO &object):m_hIO(NULL){ Attach(object.m_hIO); }

	void Attach(HMMIO handle){ assert(m_hIO==NULL); m_hIO=handle; }
	void Detach(){ assert(m_hIO!=NULL); m_hIO=NULL; }

	inline FOURCC StringToFOURCC( IN LPCTSTR sz, IN UINT uFlags){ return mmioStringToFOURCC(sz, uFlags); }
	inline LPMMIOPROC InstallIOProc( IN FOURCC fccIOProc, IN LPMMIOPROC pIOProc, IN DWORD dwFlags){ return mmioInstallIOProc(fccIOProc, pIOProc, dwFlags); }

	inline BOOL Open( IN OUT LPTSTR pszFileName, IN OUT LPMMIOINFO pmmioinfo = NULL, IN DWORD fdwOpen = MMIO_READ){
		assert(m_hIO == NULL);
		m_hIO = mmioOpen(pszFileName, pmmioinfo, fdwOpen);
		return m_hIO ? TRUE : FALSE;
	}

	inline MMRESULT Rename( IN LPCTSTR pszFileName, IN LPCTSTR pszNewFileName, IN LPCMMIOINFO pmmioinfo, IN DWORD fdwRename){
		return mmioRename(pszFileName, pszNewFileName, pmmioinfo, fdwRename);
	}

	inline MMRESULT Close( IN UINT fuClose = MMIO_FHOPEN){
		assert(m_hIO != NULL);
		MMRESULT result = mmioClose(m_hIO, fuClose);
		if (result == MMSYSERR_NOERROR)
			m_hIO = NULL;
		return result;
	}

	inline LONG Read( OUT HPSTR pch, IN LONG cch){
		assert(m_hIO != NULL);
		return mmioRead(m_hIO, pch, cch);
	}

	inline LONG Write( IN const char _huge* pch, IN LONG cch){
		assert(m_hIO != NULL);
		return mmioWrite(m_hIO, pch, cch);
	}

	inline LONG Seek( IN LONG lOffset, IN int iOrigin = SEEK_SET){
		assert(m_hIO != NULL);
		return mmioSeek(m_hIO, lOffset, iOrigin);
	}

	inline MMRESULT GetInfo( OUT LPMMIOINFO pmmioinfo, IN UINT fuInfo){
		assert(m_hIO != NULL);
		return mmioGetInfo(m_hIO, pmmioinfo, fuInfo);
	}

	inline MMRESULT SetInfo( IN LPCMMIOINFO pmmioinfo, IN UINT fuInfo){
		assert(m_hIO != NULL);
		return mmioSetInfo(m_hIO, pmmioinfo, fuInfo);
	}

	inline MMRESULT SetBuffer( IN LPSTR pchBuffer, IN LONG cchBuffer, IN UINT fuBuffer){
		assert(m_hIO != NULL);
		return mmioSetBuffer(m_hIO, pchBuffer, cchBuffer, fuBuffer);
	}

	inline MMRESULT Flush( IN UINT fuFlush){
		assert(m_hIO != NULL);
		return mmioFlush(m_hIO, fuFlush);
	}

	inline MMRESULT Advance( IN OUT LPMMIOINFO pmmioinfo, IN UINT fuAdvance){
		assert(m_hIO != NULL);
		return mmioAdvance(m_hIO, pmmioinfo, fuAdvance);
	}

	inline LRESULT SendMessage( IN UINT uMsg, IN LPARAM lParam1, IN LPARAM lParam2){
		assert(m_hIO != NULL);
		return mmioSendMessage(m_hIO, uMsg, lParam1, lParam2);
	}

	inline MMRESULT Descend( IN OUT LPMMCKINFO pmmcki, IN const MMCKINFO FAR* pmmckiParent, IN UINT fuDescend){
		assert(m_hIO != NULL);
		return mmioDescend(m_hIO, pmmcki, pmmckiParent, fuDescend);
	}

	inline MMRESULT Ascend( IN LPMMCKINFO pmmcki, IN UINT fuAscend){
		assert(m_hIO != NULL);
		return mmioAscend(m_hIO, pmmcki, fuAscend);
	}

	inline MMRESULT CreateChunk( IN LPMMCKINFO pmmcki, IN UINT fuCreate){
		assert(m_hIO != NULL);
		return mmioCreateChunk(m_hIO, pmmcki, fuCreate);
	}

};


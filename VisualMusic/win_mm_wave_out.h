
// win_mm_wave_out.h

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
 public:
  CWinMMWaveOut():m_h_wava_out_(NULL) {}
  ~CWinMMWaveOut() {if (m_h_wava_out_) Close(); assert(m_h_wava_out_==NULL);}

  explicit CWinMMWaveOut(HWAVEOUT handle)
      : m_h_wava_out_(NULL) { Attach(handle);}
  explicit CWinMMWaveOut(const CWinMMWaveOut &object):m_h_wava_out_(NULL) {
    Attach(object.m_h_wava_out_);
  }

  void Attach(HWAVEOUT handle) {
    assert(m_h_wava_out_==NULL); m_h_wava_out_=handle;
  }
  void Detach() { assert(m_h_wava_out_!=NULL); m_h_wava_out_=NULL; }

  static UINT GetNumDevs();
  static MMRESULT GetDevCaps(UINT_PTR uDeviceID,
                             LPWAVEOUTCAPS pwoc, UINT cbwoc);

  BOOL GetVolume(LPDWORD pdwVolume);
  BOOL GetVolume(DWORD dwVolume);

  BOOL Open(LPCWAVEFORMATEX pwfx, UINT uDeviceID);
  BOOL Open(LPCWAVEFORMATEX pwfx, WAVEOUTCALLBACKPROC waveOutProc,
            DWORD_PTR dwInstance, UINT uDeviceID);
  BOOL Open(LPCWAVEFORMATEX pwfx, HWND hWindow, UINT uDeviceID);
  BOOL Open(LPCWAVEFORMATEX pwfx, DWORD dwThreadID, DWORD_PTR dwInstance,
            UINT uDeviceID);
  BOOL Open(LPCWAVEFORMATEX pwfx, HANDLE hEvent, DWORD_PTR dwInstance,
            UINT uDeviceID);
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

 private:
  HWAVEOUT m_h_wava_out_;
  MMRESULT mmrLastResult;

};



class CWinMMIO {
 public:
  CWinMMIO():m_hIO(NULL) {}
  ~CWinMMIO() {if (m_hIO) Close(); assert(m_hIO==NULL);}

  explicit CWinMMIO(HMMIO handle):m_hIO(NULL) { Attach(handle);}
  explicit CWinMMIO(const CWinMMIO &object)
      :m_hIO(NULL) { Attach(object.m_hIO); }

  void Attach(HMMIO handle) { assert(m_hIO==NULL); m_hIO=handle; }
  void Detach() { assert(m_hIO!=NULL); m_hIO=NULL; }

  inline FOURCC StringToFOURCC( IN LPCTSTR sz, IN UINT uFlags) {
    return mmioStringToFOURCC(sz, uFlags);
  }
  inline LPMMIOPROC InstallIOProc( IN FOURCC fccIOProc, IN LPMMIOPROC pIOProc,
                                   IN DWORD dwFlags) {
    return mmioInstallIOProc(fccIOProc, pIOProc, dwFlags);
  }

  inline BOOL Open( IN OUT LPTSTR pszFileName,
                    IN OUT LPMMIOINFO pmmioinfo = NULL,
                    IN DWORD fdwOpen = MMIO_READ) {
    assert(m_hIO == NULL);
    m_hIO = mmioOpen(pszFileName, pmmioinfo, fdwOpen);
    return m_hIO ? TRUE : FALSE;
  }

  inline MMRESULT Rename( IN LPCTSTR pszFileName, IN LPCTSTR pszNewFileName,
                          IN LPCMMIOINFO pmmioinfo, IN DWORD fdwRename) {
    return mmioRename(pszFileName, pszNewFileName, pmmioinfo, fdwRename);
  }

  inline MMRESULT Close( IN UINT fuClose = MMIO_FHOPEN) {
    assert(m_hIO != NULL);
    MMRESULT result = mmioClose(m_hIO, fuClose);
    if (result == MMSYSERR_NOERROR)
      m_hIO = NULL;
    return result;
  }

  inline LONG Read( OUT HPSTR pch, IN LONG cch) {
    assert(m_hIO != NULL);
    return mmioRead(m_hIO, pch, cch);
  }

  inline LONG Write( IN const char _huge* pch, IN LONG cch) {
    assert(m_hIO != NULL);
    return mmioWrite(m_hIO, pch, cch);
  }

  inline LONG Seek( IN LONG lOffset, IN int iOrigin = SEEK_SET) {
    assert(m_hIO != NULL);
    return mmioSeek(m_hIO, lOffset, iOrigin);
  }

  inline MMRESULT GetInfo( OUT LPMMIOINFO pmmioinfo, IN UINT fuInfo) {
    assert(m_hIO != NULL);
    return mmioGetInfo(m_hIO, pmmioinfo, fuInfo);
  }

  inline MMRESULT SetInfo( IN LPCMMIOINFO pmmioinfo, IN UINT fuInfo) {
    assert(m_hIO != NULL);
    return mmioSetInfo(m_hIO, pmmioinfo, fuInfo);
  }

  inline MMRESULT SetBuffer( IN LPSTR pchBuffer, IN LONG cchBuffer,
                             IN UINT fuBuffer) {
    assert(m_hIO != NULL);
    return mmioSetBuffer(m_hIO, pchBuffer, cchBuffer, fuBuffer);
  }

  inline MMRESULT Flush( IN UINT fuFlush) {
    assert(m_hIO != NULL);
    return mmioFlush(m_hIO, fuFlush);
  }

  inline MMRESULT Advance( IN OUT LPMMIOINFO pmmioinfo, IN UINT fuAdvance) {
    assert(m_hIO != NULL);
    return mmioAdvance(m_hIO, pmmioinfo, fuAdvance);
  }

  inline LRESULT SendMessage( IN UINT uMsg, IN LPARAM lParam1,
                              IN LPARAM lParam2) {
    assert(m_hIO != NULL);
    return mmioSendMessage(m_hIO, uMsg, lParam1, lParam2);
  }

  inline MMRESULT Descend( IN OUT LPMMCKINFO pmmcki,
                           IN const MMCKINFO FAR* pmmckiParent,
                           IN UINT fuDescend) {
    assert(m_hIO != NULL);
    return mmioDescend(m_hIO, pmmcki, pmmckiParent, fuDescend);
  }

  inline MMRESULT Ascend( IN LPMMCKINFO pmmcki, IN UINT fuAscend) {
    assert(m_hIO != NULL);
    return mmioAscend(m_hIO, pmmcki, fuAscend);
  }

  inline MMRESULT CreateChunk( IN LPMMCKINFO pmmcki, IN UINT fuCreate) {
    assert(m_hIO != NULL);
    return mmioCreateChunk(m_hIO, pmmcki, fuCreate);
  }

 protected:
  HMMIO m_hIO;
  static const DWORD LastErrorTab[];


};






// waveOutGetNumDevs
inline UINT CWinMMWaveOut::GetNumDevs() {
  return waveOutGetNumDevs();
}

// waveOutGetDevCaps
inline MMRESULT CWinMMWaveOut::GetDevCaps(UINT_PTR uDeviceID,
                                          LPWAVEOUTCAPS pwoc,
                                          UINT cbwoc) {
  return waveOutGetDevCaps(uDeviceID, pwoc, cbwoc);
}

// waveOutGetVolume
inline BOOL CWinMMWaveOut::GetVolume(LPDWORD pdwVolume) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutGetVolume(m_h_wava_out_, pdwVolume);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutSetVolume
inline BOOL CWinMMWaveOut::GetVolume(DWORD dwVolume) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutSetVolume(m_h_wava_out_, dwVolume);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutOpen
inline BOOL CWinMMWaveOut::Open(LPCWAVEFORMATEX pwfx,
                                UINT uDeviceID = WAVE_MAPPER) {
  assert(m_h_wava_out_ == NULL);
  mmrLastResult = waveOutOpen(
    &m_h_wava_out_,
    uDeviceID,
    pwfx,
    0,
    0,
    CALLBACK_NULL
    );
  return mmrLastResult == MMSYSERR_NOERROR;
}

inline BOOL CWinMMWaveOut::Open(LPCWAVEFORMATEX pwfx,
                WAVEOUTCALLBACKPROC waveOutProc,
                DWORD_PTR dwInstance = 0,
                UINT uDeviceID = WAVE_MAPPER
                )
{
  assert(m_h_wava_out_ == NULL);
  mmrLastResult = waveOutOpen(
    &m_h_wava_out_,
    uDeviceID,
    pwfx,
    (DWORD_PTR)waveOutProc,
    dwInstance,
    CALLBACK_FUNCTION
    );
  return mmrLastResult == MMSYSERR_NOERROR;
}

inline BOOL CWinMMWaveOut::Open(LPCWAVEFORMATEX pwfx,
                HWND hWindow,
                UINT uDeviceID = WAVE_MAPPER
                )
{
  assert(m_h_wava_out_ == NULL);
  mmrLastResult = waveOutOpen(
    &m_h_wava_out_,
    uDeviceID,
    pwfx,
    (DWORD_PTR)hWindow,
    0,
    CALLBACK_WINDOW
    );
  return mmrLastResult == MMSYSERR_NOERROR;
}

inline BOOL CWinMMWaveOut::Open(LPCWAVEFORMATEX pwfx,
                DWORD dwThreadID,
                DWORD_PTR dwInstance = 0,
                UINT uDeviceID = WAVE_MAPPER
                )
{
  assert(m_h_wava_out_ == NULL);
  mmrLastResult = waveOutOpen(
    &m_h_wava_out_,
    uDeviceID,
    pwfx,
    (DWORD_PTR)dwThreadID,
    dwInstance,
    CALLBACK_THREAD
    );
  return mmrLastResult == MMSYSERR_NOERROR;
}

inline BOOL CWinMMWaveOut::Open(LPCWAVEFORMATEX pwfx,
                HANDLE hEvent,
                DWORD_PTR dwInstance = 0,
                UINT uDeviceID = WAVE_MAPPER
                )
{
  assert(m_h_wava_out_ == NULL);
  mmrLastResult = waveOutOpen(
    &m_h_wava_out_,
    uDeviceID,
    pwfx,
    (DWORD_PTR)hEvent,
    dwInstance,
    CALLBACK_EVENT
    );
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutClose
inline BOOL CWinMMWaveOut::Close() {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutClose(m_h_wava_out_);
  m_h_wava_out_ = NULL;
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutPrepareHeader
inline BOOL CWinMMWaveOut::PrepareHeader(LPWAVEHDR pwh, UINT cbwh) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutPrepareHeader(m_h_wava_out_, pwh, cbwh);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutUnprepareHeader
inline BOOL CWinMMWaveOut::UnprepareHeader(LPWAVEHDR pwh, UINT cbwh) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutUnprepareHeader(m_h_wava_out_, pwh, cbwh);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutWrite
inline BOOL CWinMMWaveOut::Write(LPWAVEHDR pwh, UINT cbwh) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutWrite(m_h_wava_out_, pwh, cbwh);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutPause
inline BOOL CWinMMWaveOut::Pause() {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutPause(m_h_wava_out_);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutRestart
inline BOOL CWinMMWaveOut::Restart() {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutRestart(m_h_wava_out_);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutReset
inline BOOL CWinMMWaveOut::Reset() {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutReset(m_h_wava_out_);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutBreakLoop
inline BOOL CWinMMWaveOut::BreakLoop() {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutBreakLoop(m_h_wava_out_);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutGetPosition
inline BOOL CWinMMWaveOut::GetPosition(LPMMTIME pmmt, UINT cbmmt) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutGetPosition(m_h_wava_out_, pmmt, cbmmt);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutGetPitch
inline BOOL CWinMMWaveOut::GetPitch(LPDWORD pdwPitch) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutGetPitch(m_h_wava_out_, pdwPitch);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutSetPitch
inline BOOL CWinMMWaveOut::SetPitch(DWORD dwPitch) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutSetPitch(m_h_wava_out_, dwPitch);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutGetPlaybackRate
inline BOOL CWinMMWaveOut::GetPlaybackRate(LPDWORD pdwRate) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutGetPlaybackRate(m_h_wava_out_, pdwRate);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutSetPlaybackRate
inline BOOL CWinMMWaveOut::SetPlaybackRate(DWORD dwRate) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutSetPlaybackRate(m_h_wava_out_, dwRate);
  return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutGetID
inline BOOL CWinMMWaveOut::GetID(LPUINT puDeviceID) {
  assert(m_h_wava_out_ != NULL);
  mmrLastResult = waveOutGetID(m_h_wava_out_, puDeviceID);
  return mmrLastResult == MMSYSERR_NOERROR;
}


inline MMRESULT CWinMMWaveOut::GetLastResult() {
  return mmrLastResult;
}

inline BOOL CWinMMWaveOut::GetErrorText(MMRESULT mmrError, LPTSTR pszText,
                                        UINT cchText) {
  return waveOutGetErrorText(mmrError,pszText,cchText) == MMSYSERR_NOERROR;
}




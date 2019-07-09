
// WinMMwaveOut.inl


// waveOutGetNumDevs
inline UINT CWinMMWaveOut::GetNumDevs(){
	return waveOutGetNumDevs();
}

// waveOutGetDevCaps
inline MMRESULT CWinMMWaveOut::GetDevCaps(UINT_PTR uDeviceID, LPWAVEOUTCAPS pwoc, UINT cbwoc){
	return waveOutGetDevCaps(uDeviceID, pwoc, cbwoc);
}

// waveOutGetVolume
inline BOOL CWinMMWaveOut::GetVolume(LPDWORD pdwVolume){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutGetVolume(m_hWavaOut, pdwVolume);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutSetVolume
inline BOOL CWinMMWaveOut::GetVolume(DWORD dwVolume){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutSetVolume(m_hWavaOut, dwVolume);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutOpen
inline BOOL CWinMMWaveOut::Open(LPCWAVEFORMATEX pwfx, UINT uDeviceID = WAVE_MAPPER){
	assert(m_hWavaOut == NULL);
	mmrLastResult = waveOutOpen(
		&m_hWavaOut,
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
	assert(m_hWavaOut == NULL);
	mmrLastResult = waveOutOpen(
		&m_hWavaOut,
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
	assert(m_hWavaOut == NULL);
	mmrLastResult = waveOutOpen(
		&m_hWavaOut,
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
	assert(m_hWavaOut == NULL);
	mmrLastResult = waveOutOpen(
		&m_hWavaOut,
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
	assert(m_hWavaOut == NULL);
	mmrLastResult = waveOutOpen(
		&m_hWavaOut,
		uDeviceID,
		pwfx,
		(DWORD_PTR)hEvent,
		dwInstance,
		CALLBACK_EVENT
		);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutClose
inline BOOL CWinMMWaveOut::Close(){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutClose(m_hWavaOut);
	m_hWavaOut = NULL;
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutPrepareHeader
inline BOOL CWinMMWaveOut::PrepareHeader(LPWAVEHDR pwh, UINT cbwh){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutPrepareHeader(m_hWavaOut, pwh, cbwh);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutUnprepareHeader
inline BOOL CWinMMWaveOut::UnprepareHeader(LPWAVEHDR pwh, UINT cbwh){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutUnprepareHeader(m_hWavaOut, pwh, cbwh);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutWrite
inline BOOL CWinMMWaveOut::Write(LPWAVEHDR pwh, UINT cbwh){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutWrite(m_hWavaOut, pwh, cbwh);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutPause
inline BOOL CWinMMWaveOut::Pause(){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutPause(m_hWavaOut);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutRestart
inline BOOL CWinMMWaveOut::Restart(){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutRestart(m_hWavaOut);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutReset
inline BOOL CWinMMWaveOut::Reset(){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutReset(m_hWavaOut);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutBreakLoop
inline BOOL CWinMMWaveOut::BreakLoop(){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutBreakLoop(m_hWavaOut);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutGetPosition
inline BOOL CWinMMWaveOut::GetPosition(LPMMTIME pmmt, UINT cbmmt){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutGetPosition(m_hWavaOut, pmmt, cbmmt);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutGetPitch
inline BOOL CWinMMWaveOut::GetPitch(LPDWORD pdwPitch){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutGetPitch(m_hWavaOut, pdwPitch);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutSetPitch
inline BOOL CWinMMWaveOut::SetPitch(DWORD dwPitch){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutSetPitch(m_hWavaOut, dwPitch);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutGetPlaybackRate
inline BOOL CWinMMWaveOut::GetPlaybackRate(LPDWORD pdwRate){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutGetPlaybackRate(m_hWavaOut, pdwRate);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutSetPlaybackRate
inline BOOL CWinMMWaveOut::SetPlaybackRate(DWORD dwRate){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutSetPlaybackRate(m_hWavaOut, dwRate);
	return mmrLastResult == MMSYSERR_NOERROR;
}

// waveOutGetID
inline BOOL CWinMMWaveOut::GetID(LPUINT puDeviceID){
	assert(m_hWavaOut != NULL);
	mmrLastResult = waveOutGetID(m_hWavaOut, puDeviceID);
	return mmrLastResult == MMSYSERR_NOERROR;
}


inline MMRESULT CWinMMWaveOut::GetLastResult(){
	return mmrLastResult;
}

inline BOOL CWinMMWaveOut::GetErrorText(MMRESULT mmrError, LPTSTR pszText, UINT cchText){
	return waveOutGetErrorText(mmrError,pszText,cchText) == MMSYSERR_NOERROR;
}



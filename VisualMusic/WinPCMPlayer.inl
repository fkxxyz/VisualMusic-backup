
// WinPCMPlayer.inl

template <int nBufferNumber>
inline CPCMPlayer<nBufferNumber>::CPCMPlayer():
	dwStatus(PCM_CLOSED)
{
	assert(nBufferNumber >= 2);
}

template <int nBufferNumber>
inline CPCMPlayer<nBufferNumber>::~CPCMPlayer(){
	
}

template <int nBufferNumber>
inline void CALLBACK CPCMPlayer<nBufferNumber>::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2){
	CPCMPlayer<nBufferNumber> &Obj = *(CPCMPlayer<nBufferNumber> *)dwInstance;

	switch (uMsg){
	case MM_WOM_OPEN:
		break;
	case MM_WOM_CLOSE:
		break;
	case MM_WOM_DONE:
		Obj.AHeaderDone((PWAVEHDR)dwParam1);
		break;
	}
}


// ���豸
template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Open(WAVEFORMATEX *pFormat){
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);

	assert(dwStatus == PCM_CLOSED);

	stWaveFormat = *pFormat;

	// ��鲨�θ�ʽ����
	if (!ValidWaveFormat())
		return FALSE;

	// ��ʼ���¼�����
	if (!AHeaderDoneEvent && !AHeaderDoneEvent.Create())
		return FALSE;
	if (!AllHeaderIdle && !AllHeaderIdle.Create())
		return FALSE;

	// �򿪲����豸
	if (!objMMWO.Open(&stWaveFormat, (WAVEOUTCALLBACKPROC)&waveOutProc, (DWORD_PTR)this))
		return FALSE;

	// ��ʼ�����в���ͷ��
	ZeroMemory(header, sizeof(header));
	for (int i = 0; i < nBufferNumber; i++){
		if (!objMMWO.PrepareHeader(&header[i], sizeof(WAVEHDR)))
			return FALSE;
	}

	// �����¼�����
	nIdleHeader = nBufferNumber;
	if (!AHeaderDoneEvent.Reset())
		return FALSE;
	if (!AllHeaderIdle.Set())
		return FALSE;

	dwStatus = PCM_READY;
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Close(){
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);
	assert(dwStatus == PCM_READY);

	for (int i = 0; i < nBufferNumber; i++){
		if (!objMMWO.UnprepareHeader(&header[i], sizeof(WAVEHDR)))
			return FALSE;
	}

	// �رղ����豸
	if (!objMMWO.Close())
		return FALSE;

	dwStatus = PCM_CLOSED;
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Clear(){
	if (dwStatus > PCM_READY)
		if (!Reset()) return FALSE;
	if (dwStatus == PCM_READY)
		if (!Close()) return FALSE;
	return TRUE;
}

template <int nBufferNumber>
inline PWAVEHDR CPCMPlayer<nBufferNumber>::WaitAndGetIdleHeader(){
	assert(dwStatus != PCM_CLOSED);
	assert(nIdleHeader >= 0);

	if (nIdleHeader == 0){
		if (!WaitAPcmDone())
			return NULL;
	}

	assert(nIdleHeader != 0);
	for (int i = 0; i < nBufferNumber; i++){
		if (!(header[i].dwFlags & WHDR_INQUEUE)){
			return &header[i];
		}
	}
	assert(0);
	return NULL;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Insert(void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes){
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);
	assert(dwStatus != PCM_CLOSED);
	assert(PcmBuffer);
	assert(dwPcmSize > 0);

	// ��ȡһ�����е�ͷ��
	PWAVEHDR pHeader = WaitAndGetIdleHeader();

	// ����������е�ͷ��
	SetHeader(pHeader, PcmBuffer, dwPcmSize, dwLoopTimes);

	if (!AllHeaderIdle.Reset())
		return FALSE;

	// ����ͷ���������
	if (!objMMWO.Write(pHeader, sizeof(WAVEHDR)))
		return FALSE;
	assert(pHeader->dwFlags & WHDR_INQUEUE);

	// �������ͷ������
	nIdleHeader--;
	assert(nIdleHeader >= 0);

	dwStatus = PCM_PLAYING;
	return TRUE;
}


template <int nBufferNumber>
inline void CPCMPlayer<nBufferNumber>::SetHeader(PWAVEHDR pHeader, void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes){
	assert(!(pHeader->dwFlags & WHDR_INQUEUE));
	assert(PcmBuffer);

	pHeader->lpData = (LPSTR)PcmBuffer;
	pHeader->dwBufferLength = dwPcmSize;
	if (dwLoopTimes < 2){
		// �����ѭ������ȥ��ѭ����־λ
		pHeader->dwFlags &= 0x1F ^ (WHDR_BEGINLOOP | WHDR_ENDLOOP);
	} else {
		// ѭ��������ѭ��
		pHeader->dwLoops = dwLoopTimes;
		pHeader->dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
	}
}

template <int nBufferNumber>
inline void CPCMPlayer<nBufferNumber>::AHeaderDone(PWAVEHDR pHeader){
	// ���ÿ���ͷ������
	nIdleHeader++;
	assert(nIdleHeader <= nBufferNumber);

	// ���ݿ���ͷ��������������¼�
	if (nIdleHeader == nBufferNumber){
		dwStatus = PCM_READY;
		if (!AllHeaderIdle.Set())
			assert(0);
	} else { // nIdleHeader < nBufferNumber
		assert(!AllHeaderIdle.IsSeted());
	}
	if (!AHeaderDoneEvent.Set())
		assert(0);
	if (!AHeaderDoneEvent.Reset())
		assert(0);
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::WaitAPcmDone(){
	assert(dwStatus == PCM_PLAYING || dwStatus == PCM_PAUSE);
	if (!AHeaderDoneEvent.Wait()){
		assert(0);
		return FALSE;
	}
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Wait(){
	assert(dwStatus == PCM_PLAYING || dwStatus == PCM_PAUSE);

	if (!AllHeaderIdle.Wait())
		return FALSE;

	assert(dwStatus == PCM_READY);
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Pause(){
	assert(dwStatus == PCM_PLAYING);
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);
	if (dwStatus == PCM_PAUSE) return TRUE;

	if (!objMMWO.Pause())
		return FALSE;

	dwStatus = PCM_PAUSE;
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Resume(){
	assert(dwStatus == PCM_PAUSE);
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);
	if (dwStatus == PCM_PLAYING) return TRUE;

	if (!objMMWO.Restart())
		return FALSE;

	dwStatus = PCM_PLAYING;
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Reset(){
	assert(dwStatus != PCM_CLOSED);
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);

	if (!objMMWO.Reset())
		return FALSE;

	return dwStatus == PCM_READY;
}

template <int nBufferNumber>
inline DWORD CPCMPlayer<nBufferNumber>::GetStatus(){
	return dwStatus;
}

template <int nBufferNumber>
inline MMRESULT CPCMPlayer<nBufferNumber>::GetLastResult(){
	return objMMWO.GetLastResult();
}

template <int nBufferNumber>
inline DWORD CPCMPlayer<nBufferNumber>::GetBytesPos(){
	assert(dwStatus != PCM_CLOSED);

	MMTIME stMMTime;
	if (!objMMWO.GetPosition(&stMMTime, sizeof(stMMTime))){
		assert(0);
		return -1;
	}
	return stMMTime.u.cb;
}

template <int nBufferNumber>
inline DWORD CPCMPlayer<nBufferNumber>::GetTimePos(){
	return (DWORD)((LONGLONG)GetBytesPos() * 1000 / stWaveFormat.nAvgBytesPerSec);
}

template <int nBufferNumber>
inline CWinMMWaveOut *CPCMPlayer<nBufferNumber>::GetMMwaveOut(){
	return &objMMWO;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::ValidWaveFormat(){
	// ����Ϊ pcm ��ʽ
	if (stWaveFormat.wFormatTag != WAVE_FORMAT_PCM && stWaveFormat.wFormatTag != 3)
		return FALSE;

	// ���������
	if (stWaveFormat.nChannels < 1 || stWaveFormat.nChannels >2)
		return FALSE;

	// ��������
	if (stWaveFormat.nSamplesPerSec < 20 || stWaveFormat.nSamplesPerSec > 176400)
		return FALSE;

	// �������λ��
	if (stWaveFormat.wBitsPerSample != 8 && stWaveFormat.wBitsPerSample != 16 && stWaveFormat.wBitsPerSample != 32)
		return FALSE;

	// ������ݿ�ĵ�����
	if (
		stWaveFormat.nBlockAlign 
			!=
		(stWaveFormat.wBitsPerSample >> 3) * stWaveFormat.nChannels
		)
		return FALSE;

	// ������ݴ�������
	if (
		stWaveFormat.nAvgBytesPerSec
			!=
		stWaveFormat.nBlockAlign * stWaveFormat.nSamplesPerSec
		)
		return FALSE;

	// ��鸽������
	if (stWaveFormat.cbSize != 0)
		goto lErrBadFormat;

	return TRUE;

lErrBadFormat:
	SetLastError(ERROR_BAD_FORMAT);
	return FALSE;
}





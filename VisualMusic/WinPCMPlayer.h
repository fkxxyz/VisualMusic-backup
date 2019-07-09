
// WinPCMPlayer.h

#pragma once
#include "WinEvent.h"
#include "WinCritialSection.h"
#include "WinMMWaveOut.h"

// PCM ��������״̬
#define PCM_CLOSED		0
#define PCM_READY		1
#define PCM_PLAYING		2
#define PCM_PAUSE		3

template <int nBufferNumber>
class CPCMPlayer {
public:
	CPCMPlayer();
	~CPCMPlayer();

	// �򿪲����豸��������ǰ���úò�����
	BOOL Open(WAVEFORMATEX *pFormat);

	// �رղ����豸�������ڲ��ŵ�ʱ��ر��豸��Ҫ��ֹͣ��
	BOOL Close();

	// ǿ��ֹͣ���رա���������
	BOOL Clear();

	// ��һ�� PCM ��Ƶ���벥������
	BOOL Insert(void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes = 1);

	// �ȴ�ĳһ�� PCM �������
	BOOL WaitAPcmDone();

	// �ȴ����Ų�����ֱ�����
	BOOL Wait();

	// ��ͣ����
	BOOL Pause();

	// ��������
	BOOL Resume();

	// ֹͣ���Ų������ʱ��
	BOOL Reset();

	// ��ȡ��ǰ״̬
	DWORD GetStatus();

	// ��ȡ��ǰ���ŵ�λ�ã��ֽڣ�
	DWORD GetBytesPos();

	// ��ȡ��ǰ���ŵ�λ�ã����룩
	DWORD GetTimePos();

	// ��ȡ��һ������Ľ��
	MMRESULT GetLastResult();

	// ��ȡ��ǰ������ �����������
	CWinMMWaveOut *GetMMwaveOut();

private:
	WAVEFORMATEX stWaveFormat;
	CWinMMWaveOut objMMWO;

	WAVEHDR header[nBufferNumber];
	int nIdleHeader;
	// ����ͷ��
	void SetHeader(PWAVEHDR pHeader, void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes = 1);

	// �ؼ��ζ���ÿ�������ĺ��������ٽ��������ɶ��߳�ͬʱִ�ж�����
	CCriticalSection m_CriticalSection;

	// waveOut �ص�������ÿ�β�����һ������ͷ��ִ�У�
	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	void AHeaderDone(PWAVEHDR pHeader);

	CEvent AHeaderDoneEvent; // һ�� header ������ɺ��Ѿ�����ʱ����һ��Ȼ������
	CEvent AllHeaderIdle; // ���� header ������ɺ��Ѿ�����ʱ����
	PWAVEHDR WaitAndGetIdleHeader();  // ��ȡһ�����е� header�����û�п��е���ȴ������ٻ�ȡ

	DWORD dwStatus;
	BOOL ValidWaveFormat();
};

#include "WinPCMPlayer.inl"

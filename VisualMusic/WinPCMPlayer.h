
// WinPCMPlayer.h

#pragma once
#include "WinEvent.h"
#include "WinCritialSection.h"
#include "WinMMWaveOut.h"

// PCM 播放器的状态
#define PCM_CLOSED		0
#define PCM_READY		1
#define PCM_PLAYING		2
#define PCM_PAUSE		3

template <int nBufferNumber>
class CPCMPlayer {
public:
	CPCMPlayer();
	~CPCMPlayer();

	// 打开播放设备（必须提前设置好参数）
	BOOL Open(WAVEFORMATEX *pFormat);

	// 关闭播放设备（不能在播放的时候关闭设备，要先停止）
	BOOL Close();

	// 强制停止并关闭、清理播放器
	BOOL Clear();

	// 将一段 PCM 音频加入播放序列
	BOOL Insert(void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes = 1);

	// 等待某一个 PCM 播放完成
	BOOL WaitAPcmDone();

	// 等待播放播放器直到完成
	BOOL Wait();

	// 暂停播放
	BOOL Pause();

	// 继续播放
	BOOL Resume();

	// 停止播放并清零计时器
	BOOL Reset();

	// 获取当前状态
	DWORD GetStatus();

	// 获取当前播放的位置（字节）
	DWORD GetBytesPos();

	// 获取当前播放的位置（毫秒）
	DWORD GetTimePos();

	// 获取上一个错误的结果
	MMRESULT GetLastResult();

	// 获取当前关联的 波形输出对象
	CWinMMWaveOut *GetMMwaveOut();

private:
	WAVEFORMATEX stWaveFormat;
	CWinMMWaveOut objMMWO;

	WAVEHDR header[nBufferNumber];
	int nIdleHeader;
	// 设置头部
	void SetHeader(PWAVEHDR pHeader, void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes = 1);

	// 关键段对象（每个动作的函数都是临界区，不可多线程同时执行多个命令）
	CCriticalSection m_CriticalSection;

	// waveOut 回调函数（每次播放完一个波形头后执行）
	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	void AHeaderDone(PWAVEHDR pHeader);

	CEvent AHeaderDoneEvent; // 一个 header 播放完成后已经空闲时触发一次然后重置
	CEvent AllHeaderIdle; // 所有 header 播放完成后已经空闲时触发
	PWAVEHDR WaitAndGetIdleHeader();  // 获取一个空闲的 header，如果没有空闲的则等待空闲再获取

	DWORD dwStatus;
	BOOL ValidWaveFormat();
};

#include "WinPCMPlayer.inl"

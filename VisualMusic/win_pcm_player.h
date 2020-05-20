
// win_pcm_player.h

#pragma once
#include "win_event.h"
#include "win_critial_section.h"
#include "win_mm_wave_out.h"

// PCM 播放器的状态
#define PCM_CLOSED    0
#define PCM_READY    1
#define PCM_PLAYING    2
#define PCM_PAUSE    3

template <int nBufferNumber>
class CPCMPlayer {
 public:
  CPCMPlayer();
  ~CPCMPlayer();

  // 打开播放设备（必须提前设置好参数）
  BOOL Open(WAVEFORMATEX *p_format);

  // 关闭播放设备（不能在播放的时候关闭设备，要先停止）
  BOOL Close();

  // 强制停止并关闭、清理播放器
  BOOL Clear();

  // 将一段 PCM 音频加入播放序列
  BOOL Insert(void *pcm_buffer, DWORD dw_pcm_size, DWORD dw_loop_times = 1);

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
  WAVEFORMATEX st_wave_format_;
  CWinMMWaveOut obj_mm_wave_out_;

  WAVEHDR header_[nBufferNumber];
  int num_idle_header_;
  // 设置头部
  void SetHeader(PWAVEHDR p_header, void *pcm_buffer,
                 DWORD dw_pcm_size, DWORD dw_loop_times = 1);

  // 关键段对象（每个动作的函数都是临界区，不可多线程同时执行多个命令）
  CCriticalSection m_critical_section_;

  // waveOut 回调函数（每次播放完一个波形头后执行）
  static void CALLBACK waveOutProc(
      HWAVEOUT hwo, UINT uMsg,
      DWORD_PTR dwInstance,
      DWORD_PTR dwParam1,
      DWORD_PTR dwParam2);
  void AHeaderDone(PWAVEHDR p_header);

  CEvent a_header_done_event_; // 一个 header_ 播放完成后已经空闲时触发一次然后重置
  CEvent all_header_idle_; // 所有 header_ 播放完成后已经空闲时触发
  PWAVEHDR WaitAndGetIdleHeader();  // 获取一个空闲的 header_，如果没有空闲的则等待空闲再获取

  DWORD dw_status_;
  BOOL ValidWaveFormat();
};




template <int nBufferNumber>
inline CPCMPlayer<nBufferNumber>::CPCMPlayer():
  dw_status_(PCM_CLOSED)
{
  assert(nBufferNumber >= 2);
}

template <int nBufferNumber>
inline CPCMPlayer<nBufferNumber>::~CPCMPlayer() {
  
}

template <int nBufferNumber>
inline void CALLBACK CPCMPlayer<nBufferNumber>::waveOutProc(
    HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance,
    DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
  CPCMPlayer<nBufferNumber> &Obj = *(CPCMPlayer<nBufferNumber> *)dwInstance;

  switch (uMsg) {
  case MM_WOM_OPEN:
    break;
  case MM_WOM_CLOSE:
    break;
  case MM_WOM_DONE:
    Obj.AHeaderDone((PWAVEHDR)dwParam1);
    break;
  }
}


// 打开设备
template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Open(WAVEFORMATEX *p_format) {
  CCriticalSectionFunction objCriticalFunction(m_critical_section_);

  assert(dw_status_ == PCM_CLOSED);

  st_wave_format_ = *p_format;

  // 检查波形格式参数
  if (!ValidWaveFormat())
    return FALSE;

  // 初始化事件对象
  if (!a_header_done_event_ && !a_header_done_event_.Create())
    return FALSE;
  if (!all_header_idle_ && !all_header_idle_.Create())
    return FALSE;

  // 打开波形设备
  if (!obj_mm_wave_out_.Open(&st_wave_format_,
      (WAVEOUTCALLBACKPROC)&waveOutProc, (DWORD_PTR)this))
    return FALSE;

  // 初始化所有波形头部
  ZeroMemory(header_, sizeof(header_));
  for (int i = 0; i < nBufferNumber; i++) {
    if (!obj_mm_wave_out_.PrepareHeader(&header_[i], sizeof(WAVEHDR)))
      return FALSE;
  }

  // 处理事件对象
  num_idle_header_ = nBufferNumber;
  if (!a_header_done_event_.Reset())
    return FALSE;
  if (!all_header_idle_.Set())
    return FALSE;

  dw_status_ = PCM_READY;
  return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Close() {
  CCriticalSectionFunction objCriticalFunction(m_critical_section_);
  assert(dw_status_ == PCM_READY);

  for (int i = 0; i < nBufferNumber; i++) {
    if (!obj_mm_wave_out_.UnprepareHeader(&header_[i], sizeof(WAVEHDR)))
      return FALSE;
  }

  // 关闭波形设备
  if (!obj_mm_wave_out_.Close())
    return FALSE;

  dw_status_ = PCM_CLOSED;
  return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Clear() {
  if (dw_status_ > PCM_READY)
    if (!Reset()) return FALSE;
  if (dw_status_ == PCM_READY)
    if (!Close()) return FALSE;
  return TRUE;
}

template <int nBufferNumber>
inline PWAVEHDR CPCMPlayer<nBufferNumber>::WaitAndGetIdleHeader() {
  assert(dw_status_ != PCM_CLOSED);
  assert(num_idle_header_ >= 0);

  if (num_idle_header_ == 0) {
    if (!WaitAPcmDone())
      return NULL;
  }

  assert(num_idle_header_ != 0);
  for (int i = 0; i < nBufferNumber; i++) {
    if (!(header_[i].dwFlags & WHDR_INQUEUE)) {
      return &header_[i];
    }
  }
  assert(0);
  return NULL;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Insert(void *pcm_buffer,
                                              DWORD dw_pcm_size,
                                              DWORD dw_loop_times) {
  CCriticalSectionFunction objCriticalFunction(m_critical_section_);
  assert(dw_status_ != PCM_CLOSED);
  assert(pcm_buffer);
  assert(dw_pcm_size > 0);

  // 获取一个空闲的头部
  PWAVEHDR p_header = WaitAndGetIdleHeader();

  // 设置这个空闲的头部
  SetHeader(p_header, pcm_buffer, dw_pcm_size, dw_loop_times);

  if (!all_header_idle_.Reset())
    return FALSE;

  // 将此头部加入队列
  if (!obj_mm_wave_out_.Write(p_header, sizeof(WAVEHDR)))
    return FALSE;
  assert(p_header->dwFlags & WHDR_INQUEUE);

  // 处理空闲头部个数
  num_idle_header_--;
  assert(num_idle_header_ >= 0);

  dw_status_ = PCM_PLAYING;
  return TRUE;
}


template <int nBufferNumber>
inline void CPCMPlayer<nBufferNumber>::SetHeader(
    PWAVEHDR p_header, void *pcm_buffer,
    DWORD dw_pcm_size, DWORD dw_loop_times) {
  assert(!(p_header->dwFlags & WHDR_INQUEUE));
  assert(pcm_buffer);

  p_header->lpData = (LPSTR)pcm_buffer;
  p_header->dwBufferLength = dw_pcm_size;
  if (dw_loop_times < 2) {
    // 如果不循环，则去除循环标志位
    p_header->dwFlags &= 0x1F ^ (WHDR_BEGINLOOP | WHDR_ENDLOOP);
  } else {
    // 循环则设置循环
    p_header->dwLoops = dw_loop_times;
    p_header->dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
  }
}

template <int nBufferNumber>
inline void CPCMPlayer<nBufferNumber>::AHeaderDone(PWAVEHDR p_header) {
  // 设置空闲头部个数
  num_idle_header_++;
  assert(num_idle_header_ <= nBufferNumber);

  // 根据空闲头部个数设置相关事件
  if (num_idle_header_ == nBufferNumber) {
    dw_status_ = PCM_READY;
    if (!all_header_idle_.Set())
      assert(0);
  } else { // num_idle_header_ < nBufferNumber
    assert(!all_header_idle_.IsSeted());
  }
  if (!a_header_done_event_.Set())
    assert(0);
  if (!a_header_done_event_.Reset())
    assert(0);
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::WaitAPcmDone() {
  assert(dw_status_ == PCM_PLAYING || dw_status_ == PCM_PAUSE);
  if (!a_header_done_event_.Wait()) {
    assert(0);
    return FALSE;
  }
  return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Wait() {
  assert(dw_status_ == PCM_PLAYING || dw_status_ == PCM_PAUSE);

  if (!all_header_idle_.Wait())
    return FALSE;

  assert(dw_status_ == PCM_READY);
  return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Pause() {
  assert(dw_status_ == PCM_PLAYING);
  CCriticalSectionFunction objCriticalFunction(m_critical_section_);
  if (dw_status_ == PCM_PAUSE) return TRUE;

  if (!obj_mm_wave_out_.Pause())
    return FALSE;

  dw_status_ = PCM_PAUSE;
  return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Resume() {
  assert(dw_status_ == PCM_PAUSE);
  CCriticalSectionFunction objCriticalFunction(m_critical_section_);
  if (dw_status_ == PCM_PLAYING) return TRUE;

  if (!obj_mm_wave_out_.Restart())
    return FALSE;

  dw_status_ = PCM_PLAYING;
  return TRUE;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Reset() {
  assert(dw_status_ != PCM_CLOSED);
  CCriticalSectionFunction objCriticalFunction(m_critical_section_);

  if (!obj_mm_wave_out_.Reset())
    return FALSE;

  return dw_status_ == PCM_READY;
}

template <int nBufferNumber>
inline DWORD CPCMPlayer<nBufferNumber>::GetStatus() {
  return dw_status_;
}

template <int nBufferNumber>
inline MMRESULT CPCMPlayer<nBufferNumber>::GetLastResult() {
  return obj_mm_wave_out_.GetLastResult();
}

template <int nBufferNumber>
inline DWORD CPCMPlayer<nBufferNumber>::GetBytesPos() {
  assert(dw_status_ != PCM_CLOSED);

  MMTIME stMMTime;
  if (!obj_mm_wave_out_.GetPosition(&stMMTime, sizeof(stMMTime))) {
    assert(0);
    return -1;
  }
  return stMMTime.u.cb;
}

template <int nBufferNumber>
inline DWORD CPCMPlayer<nBufferNumber>::GetTimePos() {
  return 
      (DWORD)(
          (LONGLONG)GetBytesPos() * 1000 / st_wave_format_.nAvgBytesPerSec);
}

template <int nBufferNumber>
inline CWinMMWaveOut *CPCMPlayer<nBufferNumber>::GetMMwaveOut() {
  return &obj_mm_wave_out_;
}

template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::ValidWaveFormat() {
  // 必须为 pcm 格式
  if (st_wave_format_.wFormatTag !=
      WAVE_FORMAT_PCM && st_wave_format_.wFormatTag != 3)
    return FALSE;

  // 检查声道数
  if (st_wave_format_.nChannels < 1 || st_wave_format_.nChannels >2)
    return FALSE;

  // 检查采样率
  if (st_wave_format_.nSamplesPerSec < 20 ||
      st_wave_format_.nSamplesPerSec > 176400)
    return FALSE;

  // 检查样本位数
  if (st_wave_format_.wBitsPerSample != 8 &&
      st_wave_format_.wBitsPerSample != 16 &&
      st_wave_format_.wBitsPerSample != 32)
    return FALSE;

  // 检查数据块的调整数
  if (
    st_wave_format_.nBlockAlign 
      !=
    (st_wave_format_.wBitsPerSample >> 3) * st_wave_format_.nChannels
    )
    return FALSE;

  // 检查数据传输速率
  if (
    st_wave_format_.nAvgBytesPerSec
      !=
    st_wave_format_.nBlockAlign * st_wave_format_.nSamplesPerSec
    )
    return FALSE;

  // 检查附加数据
  if (st_wave_format_.cbSize != 0)
    goto lErrBadFormat;

  return TRUE;

lErrBadFormat:
  SetLastError(ERROR_BAD_FORMAT);
  return FALSE;
}






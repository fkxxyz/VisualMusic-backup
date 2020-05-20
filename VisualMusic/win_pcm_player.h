
// win_pcm_player.h

#pragma once
#include "win_event.h"
#include "win_critial_section.h"
#include "win_mm_wave_out.h"

// PCM ��������״̬
#define PCM_CLOSED    0
#define PCM_READY    1
#define PCM_PLAYING    2
#define PCM_PAUSE    3

template <int nBufferNumber>
class CPCMPlayer {
 public:
  CPCMPlayer();
  ~CPCMPlayer();

  // �򿪲����豸��������ǰ���úò�����
  BOOL Open(WAVEFORMATEX *p_format);

  // �رղ����豸�������ڲ��ŵ�ʱ��ر��豸��Ҫ��ֹͣ��
  BOOL Close();

  // ǿ��ֹͣ���رա���������
  BOOL Clear();

  // ��һ�� PCM ��Ƶ���벥������
  BOOL Insert(void *pcm_buffer, DWORD dw_pcm_size, DWORD dw_loop_times = 1);

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
  WAVEFORMATEX st_wave_format_;
  CWinMMWaveOut obj_mm_wave_out_;

  WAVEHDR header_[nBufferNumber];
  int num_idle_header_;
  // ����ͷ��
  void SetHeader(PWAVEHDR p_header, void *pcm_buffer,
                 DWORD dw_pcm_size, DWORD dw_loop_times = 1);

  // �ؼ��ζ���ÿ�������ĺ��������ٽ��������ɶ��߳�ͬʱִ�ж�����
  CCriticalSection m_critical_section_;

  // waveOut �ص�������ÿ�β�����һ������ͷ��ִ�У�
  static void CALLBACK waveOutProc(
      HWAVEOUT hwo, UINT uMsg,
      DWORD_PTR dwInstance,
      DWORD_PTR dwParam1,
      DWORD_PTR dwParam2);
  void AHeaderDone(PWAVEHDR p_header);

  CEvent a_header_done_event_; // һ�� header_ ������ɺ��Ѿ�����ʱ����һ��Ȼ������
  CEvent all_header_idle_; // ���� header_ ������ɺ��Ѿ�����ʱ����
  PWAVEHDR WaitAndGetIdleHeader();  // ��ȡһ�����е� header_�����û�п��е���ȴ������ٻ�ȡ

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


// ���豸
template <int nBufferNumber>
inline BOOL CPCMPlayer<nBufferNumber>::Open(WAVEFORMATEX *p_format) {
  CCriticalSectionFunction objCriticalFunction(m_critical_section_);

  assert(dw_status_ == PCM_CLOSED);

  st_wave_format_ = *p_format;

  // ��鲨�θ�ʽ����
  if (!ValidWaveFormat())
    return FALSE;

  // ��ʼ���¼�����
  if (!a_header_done_event_ && !a_header_done_event_.Create())
    return FALSE;
  if (!all_header_idle_ && !all_header_idle_.Create())
    return FALSE;

  // �򿪲����豸
  if (!obj_mm_wave_out_.Open(&st_wave_format_,
      (WAVEOUTCALLBACKPROC)&waveOutProc, (DWORD_PTR)this))
    return FALSE;

  // ��ʼ�����в���ͷ��
  ZeroMemory(header_, sizeof(header_));
  for (int i = 0; i < nBufferNumber; i++) {
    if (!obj_mm_wave_out_.PrepareHeader(&header_[i], sizeof(WAVEHDR)))
      return FALSE;
  }

  // �����¼�����
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

  // �رղ����豸
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

  // ��ȡһ�����е�ͷ��
  PWAVEHDR p_header = WaitAndGetIdleHeader();

  // ����������е�ͷ��
  SetHeader(p_header, pcm_buffer, dw_pcm_size, dw_loop_times);

  if (!all_header_idle_.Reset())
    return FALSE;

  // ����ͷ���������
  if (!obj_mm_wave_out_.Write(p_header, sizeof(WAVEHDR)))
    return FALSE;
  assert(p_header->dwFlags & WHDR_INQUEUE);

  // �������ͷ������
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
    // �����ѭ������ȥ��ѭ����־λ
    p_header->dwFlags &= 0x1F ^ (WHDR_BEGINLOOP | WHDR_ENDLOOP);
  } else {
    // ѭ��������ѭ��
    p_header->dwLoops = dw_loop_times;
    p_header->dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
  }
}

template <int nBufferNumber>
inline void CPCMPlayer<nBufferNumber>::AHeaderDone(PWAVEHDR p_header) {
  // ���ÿ���ͷ������
  num_idle_header_++;
  assert(num_idle_header_ <= nBufferNumber);

  // ���ݿ���ͷ��������������¼�
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
  // ����Ϊ pcm ��ʽ
  if (st_wave_format_.wFormatTag !=
      WAVE_FORMAT_PCM && st_wave_format_.wFormatTag != 3)
    return FALSE;

  // ���������
  if (st_wave_format_.nChannels < 1 || st_wave_format_.nChannels >2)
    return FALSE;

  // ��������
  if (st_wave_format_.nSamplesPerSec < 20 ||
      st_wave_format_.nSamplesPerSec > 176400)
    return FALSE;

  // �������λ��
  if (st_wave_format_.wBitsPerSample != 8 &&
      st_wave_format_.wBitsPerSample != 16 &&
      st_wave_format_.wBitsPerSample != 32)
    return FALSE;

  // ������ݿ�ĵ�����
  if (
    st_wave_format_.nBlockAlign 
      !=
    (st_wave_format_.wBitsPerSample >> 3) * st_wave_format_.nChannels
    )
    return FALSE;

  // ������ݴ�������
  if (
    st_wave_format_.nAvgBytesPerSec
      !=
    st_wave_format_.nBlockAlign * st_wave_format_.nSamplesPerSec
    )
    return FALSE;

  // ��鸽������
  if (st_wave_format_.cbSize != 0)
    goto lErrBadFormat;

  return TRUE;

lErrBadFormat:
  SetLastError(ERROR_BAD_FORMAT);
  return FALSE;
}






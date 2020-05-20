
// cthread.h
#pragma once

#include <cstddef>
#include <cassert>

#ifdef WIN32
#ifndef _WIN32_WINNT    // 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501  // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <sys/sem.h>
#endif

/*
  跨平台的多线程 C++ 类
  CThread、CThreadProc 线程类
  CCriticalSection 互斥量类
  CSpinLock 自旋锁
  CSemaphore 信号量
*/

namespace cth {

class CThread {
 public:
  typedef void *(* ThreadFunc)(void *);

  CThread();
  ~CThread();
  int Create(ThreadFunc ThreadProc, void *param);
  void *GetID() const;
  void Join();
  int Terminate();
  int StillActive();

 protected:
#ifdef WIN32
  void *m_handle_;
#endif
  void *m_tid_;

 private:
#ifdef WIN32
  static unsigned __stdcall WinStartAddress(void *);
  ThreadFunc m_thread_proc_;
  void *m_param_;
#endif
};


class CThreadProc : public CThread {
 public:
  int Create();

 protected:
  virtual void *ThreadProc() = 0;
  static void *StaticThreadFunc(void *param);
};


class CCriticalSection {
 public:
  CCriticalSection();
  ~CCriticalSection();
  void Enter();
  void Leave();
  int TryEnter();

 protected:
#ifdef WIN32
  CRITICAL_SECTION m_h_critical_section_;
#else
  pthread_mutex_t m_mutex_;
  pthread_mutexattr_t m_attr_;
#endif
};


class CSpinLock {
 public:
  CSpinLock(CCriticalSection &CriticalSection);
  ~CSpinLock();

 protected:
  CCriticalSection &m_critical_section_;
};


class CSemaphore {
 public:
  CSemaphore(int init_num, int max_num);
  ~CSemaphore();

  void P();
  void V();

 private:
#ifdef WIN32
  HANDLE m_h_sem_;
#else
  int m_id_sem_;
#endif
};



inline CThread::CThread()
#ifdef WIN32
:m_handle_(NULL)
#endif
{
  
}

inline CThread::~CThread() {
#ifdef WIN32
  if (m_handle_)
    CloseHandle((HANDLE)m_handle_);
#endif
}

inline int CThread::Create(ThreadFunc ThreadProc, void *param) {
#ifdef WIN32
  if (m_handle_)
    CloseHandle((HANDLE)m_handle_);
  m_thread_proc_ = ThreadProc;
  m_param_ = param;
  m_handle_ = (void *)_beginthreadex(NULL, 0, WinStartAddress,
      this, 0, (unsigned int *)&m_tid_);
  if (m_handle_ == (void *)-1) m_handle_ = NULL;
#else
  int m_handle_ = !pthread_create((pthread_t *)&m_tid_,
                                  NULL, ThreadProc, param);
#endif

  return m_handle_ != NULL;
}

inline void *CThread::GetID() const{
  return m_tid_;
}

inline void CThread::Join() {
#ifdef WIN32
  if (!m_handle_) return;
  WaitForSingleObject(m_handle_, INFINITE);
  CloseHandle(m_handle_);
  m_handle_ = NULL;
#else
  pthread_join(m_tid_, NULL);
#endif
}

inline int CThread::Terminate() {
#ifdef WIN32
  return m_handle_ ? TerminateThread(m_handle_, 1) : 1;
#else
  return !pthread_cancel(m_tid_);
#endif
}

inline int CThread::StillActive() {
#ifdef WIN32
  if (!m_handle_) return 0;
  DWORD dw_exit_code;
  return GetExitCodeThread(m_handle_, &dw_exit_code) &&
      dw_exit_code == STILL_ACTIVE;
#else
  int ret = pthread_kill(m_tid_, 0);
  return (ret == ESRCH || ret == EINVAL) ? 0 : 1;
#endif
}

#ifdef WIN32
inline unsigned CThread::WinStartAddress(void *pthis) {
  CThread *p = (CThread *)pthis;
  return (unsigned)(intptr_t)p->m_thread_proc_(p->m_param_);
}
#endif



inline int CThreadProc::Create() {
  return CThread::Create(StaticThreadFunc, this);
}

inline void *CThreadProc::StaticThreadFunc(void *param) {
  return ((CThreadProc *)param)->ThreadProc();
}


inline CCriticalSection::CCriticalSection() {
#ifdef WIN32
  InitializeCriticalSection(&m_h_critical_section_);
#else
  pthread_mutexattr_init(&m_attr_);
  pthread_mutexattr_setpshared(&m_attr_, PTHREAD_PROCESS_PRIVATE);
  pthread_mutexattr_settype(&m_attr_, PTHREAD_MUTEX_NORMAL);
  pthread_mutexattr_setprotocol(&m_attr_, PTHREAD_PRIO_NONE);

  pthread_mutex_init(m_mutex_);
#endif
}

inline CCriticalSection::~CCriticalSection() {
#ifdef WIN32
  DeleteCriticalSection(&m_h_critical_section_);
#else
  pthread_mutex_destroy(&m_mutex_);
  pthread_mutexattr_destroy(&m_attr_);
#endif
}


inline void CCriticalSection::Enter() {
#ifdef WIN32
  EnterCriticalSection(&m_h_critical_section_);
#else
  pthread_mutex_lock(&m_mutex_);
#endif
}

inline void CCriticalSection::Leave() {
#ifdef WIN32
  LeaveCriticalSection(&m_h_critical_section_);
#else
  pthread_mutex_unlock(&m_mutex_);
#endif
}

inline int CCriticalSection::TryEnter() {
#ifdef WIN32
  return TryEnterCriticalSection(&m_h_critical_section_);
#else
  return pthread_mutex_trylock(&m_mutex_) == 0;
#endif
}


inline CSpinLock::CSpinLock(CCriticalSection &CriticalSection)
    : m_critical_section_(CriticalSection) {
  CriticalSection.Enter();
}

inline CSpinLock::~CSpinLock() {
  m_critical_section_.Leave();
}


inline CSemaphore::CSemaphore(int init_num, int max_num) {
#ifdef WIN32
  m_h_sem_ = CreateSemaphore(NULL, init_num, max_num, NULL);
#else
  m_id_sem_ = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
  union semun sem_union;
  sem_union.val = init_num;
  semctl(m_id_sem_, 0, SETVAL, sem_union);
#endif
}

inline CSemaphore::~CSemaphore() {
#ifdef WIN32
  CloseHandle(m_h_sem_);
#else
  union semun sem_union;
  semctl(m_id_sem_, 0, IPC_RMID, sem_union);
#endif
}


inline void CSemaphore::P() {
#ifdef WIN32
  WaitForSingleObject(m_h_sem_, INFINITE);
#else
  struct sembuf sem;
  sem.sem_num = 0;
  sem.sem_op = -1;
  semop(m_id_sem_, &sem, 1);
#endif
}

inline void CSemaphore::V() {
#ifdef WIN32
  ReleaseSemaphore(m_h_sem_, 1, NULL);
#else
  struct sembuf sem;
  sem.sem_num = 0;
  sem.sem_op = 1;
  semop(m_id_sem_, &sem, 1);
#endif
}



}
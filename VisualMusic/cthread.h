
// cthread.h
#pragma once

#include <cstddef>
#include <cassert>

#ifdef WIN32
#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
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
	void *m_handle;
#endif
	void *m_tid;

private:
#ifdef WIN32
	static unsigned __stdcall WinStartAddress(void *);
	ThreadFunc m_ThreadProc;
	void *m_param;
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
	CRITICAL_SECTION m_hCriticalSection;
#else
	pthread_mutex_t m_Mutex;
	pthread_mutexattr_t m_Attr;
#endif
};


class CSpinLock {
public:
	CSpinLock(CCriticalSection &CriticalSection);
	~CSpinLock();

protected:
	CCriticalSection &m_CriticalSection;
};


class CSemaphore {
public:
	CSemaphore(int init_num, int max_num);
	~CSemaphore();

	void P();
	void V();

private:
#ifdef WIN32
	HANDLE m_hSem;
#else
	int m_idSem;
#endif
};



inline CThread::CThread()
#ifdef WIN32
:m_handle(NULL)
#endif
{
	
}

inline CThread::~CThread(){
#ifdef WIN32
	if (m_handle)
		CloseHandle((HANDLE)m_handle);
#endif
}

inline int CThread::Create(ThreadFunc ThreadProc, void *param){
#ifdef WIN32
	if (m_handle)
		CloseHandle((HANDLE)m_handle);
	m_ThreadProc = ThreadProc;
	m_param = param;
	m_handle = (void *)_beginthreadex(NULL, 0, WinStartAddress, this, 0, (unsigned int *)&m_tid);
	if (m_handle == (void *)-1) m_handle = NULL;
#else
	int m_handle = !pthread_create((pthread_t *)&m_tid, NULL, ThreadProc, param);
#endif

	return m_handle != NULL;
}

inline void *CThread::GetID() const{
	return m_tid;
}

inline void CThread::Join(){
#ifdef WIN32
	if (!m_handle) return;
	WaitForSingleObject(m_handle, INFINITE);
	CloseHandle(m_handle);
	m_handle = NULL;
#else
	pthread_join(m_tid, NULL);
#endif
}

inline int CThread::Terminate(){
#ifdef WIN32
	return m_handle ? TerminateThread(m_handle, 1) : 1;
#else
	return !pthread_cancel(m_tid);
#endif
}

inline int CThread::StillActive(){
#ifdef WIN32
	if (!m_handle) return 0;
	DWORD dwExitCode;
	return GetExitCodeThread(m_handle, &dwExitCode) && dwExitCode == STILL_ACTIVE;
#else
	int ret = pthread_kill(m_tid, 0);
	return (ret == ESRCH || ret == EINVAL) ? 0 : 1;
#endif
}

#ifdef WIN32
inline unsigned CThread::WinStartAddress(void *pthis){
	CThread *p = (CThread *)pthis;
	return (unsigned)(intptr_t)p->m_ThreadProc(p->m_param);
}
#endif



inline int CThreadProc::Create(){
	return CThread::Create(StaticThreadFunc, this);
}

inline void *CThreadProc::StaticThreadFunc(void *param){
	return ((CThreadProc *)param)->ThreadProc();
}


inline CCriticalSection::CCriticalSection(){
#ifdef WIN32
	InitializeCriticalSection(&m_hCriticalSection);
#else
	pthread_mutexattr_init(&m_Attr);
	pthread_mutexattr_setpshared(&m_Attr, PTHREAD_PROCESS_PRIVATE);
	pthread_mutexattr_settype(&m_Attr, PTHREAD_MUTEX_NORMAL);
	pthread_mutexattr_setprotocol(&m_Attr, PTHREAD_PRIO_NONE);

	pthread_mutex_init(m_Mutex);
#endif
}

inline CCriticalSection::~CCriticalSection(){
#ifdef WIN32
	DeleteCriticalSection(&m_hCriticalSection);
#else
	pthread_mutex_destroy(&m_Mutex);
	pthread_mutexattr_destroy(&m_Attr);
#endif
}


inline void CCriticalSection::Enter(){
#ifdef WIN32
	EnterCriticalSection(&m_hCriticalSection);
#else
	pthread_mutex_lock(&m_Mutex);
#endif
}

inline void CCriticalSection::Leave(){
#ifdef WIN32
	LeaveCriticalSection(&m_hCriticalSection);
#else
	pthread_mutex_unlock(&m_Mutex);
#endif
}

inline int CCriticalSection::TryEnter(){
#ifdef WIN32
	return TryEnterCriticalSection(&m_hCriticalSection);
#else
	return pthread_mutex_trylock(&m_Mutex) == 0;
#endif
}


inline CSpinLock::CSpinLock(CCriticalSection &CriticalSection):m_CriticalSection(CriticalSection){
	CriticalSection.Enter();
}

inline CSpinLock::~CSpinLock(){
	m_CriticalSection.Leave();
}


inline CSemaphore::CSemaphore(int init_num, int max_num){
#ifdef WIN32
	m_hSem = CreateSemaphore(NULL, init_num, max_num, NULL);
#else
	m_idSem = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
	union semun sem_union;
	sem_union.val = init_num;
	semctl(m_idSem, 0, SETVAL, sem_union);
#endif
}

inline CSemaphore::~CSemaphore(){
#ifdef WIN32
	CloseHandle(m_hSem);
#else
	union semun sem_union;
	semctl(m_idSem, 0, IPC_RMID, sem_union);
#endif
}


inline void CSemaphore::P(){
#ifdef WIN32
	WaitForSingleObject(m_hSem, INFINITE);
#else
	struct sembuf sem;
	sem.sem_num = 0;
	sem.sem_op = -1;
	semop(m_idSem, &sem, 1);
#endif
}

inline void CSemaphore::V(){
#ifdef WIN32
	ReleaseSemaphore(m_hSem, 1, NULL);
#else
	struct sembuf sem;
	sem.sem_num = 0;
	sem.sem_op = 1;
	semop(m_idSem, &sem, 1);
#endif
}



}
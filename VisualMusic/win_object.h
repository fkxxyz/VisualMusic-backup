
// WinObject.h
#pragma once

#include <cassert>
#include <windows.h>

template <class HTYPE>
class CHandleObject {
 public:
  CHandleObject():m_handle_(NULL) {}
  explicit CHandleObject(HTYPE handle):m_handle_(NULL) { Attach(handle);}
  explicit CHandleObject(const CHandleObject &object)
      : m_handle_(NULL) { Attach((HTYPE)object); }

  // 必须在子类的析构函数里面调用 Close()，Close() 的功能是关闭句柄，并且设置句柄值为 NULL
  ~CHandleObject() { assert(m_handle_==NULL); }

  //virtual BOOL Open() = 0;
  //virtual BOOL Create() = 0;
  virtual BOOL Close() = 0;

  virtual void Attach(HTYPE handle) {
    assert(m_handle_==NULL);
    m_handle_=handle;
  }
  virtual void Detach() {
    assert(m_handle_!=NULL);
    m_handle_=NULL;
  }

  operator HTYPE() const { return m_handle_; }
  BOOL operator ==(CHandleObject &object) const { return m_handle_==object; }
  CHandleObject &operator =(const CHandleObject &object) {
    Attach(object); return *this;
  }

 protected:
  HTYPE m_handle_;
};

class CKernelObject : public CHandleObject<HANDLE>{
 public:
  ~CKernelObject() {
    if (m_handle_)
      Close();
  }

  //virtual BOOL Open() = 0;
  //virtual BOOL Create() = 0;

  // CloseHandle
  virtual BOOL Close() {
    assert(m_handle_!=NULL);
    if (!CloseHandle(m_handle_)) return FALSE;
    m_handle_=NULL;
    return TRUE;
  }

  // WaitForSingleObject
  virtual BOOL Wait(DWORD dwMilliseconds = INFINITE) const {
    assert(m_handle_!=NULL);
    DWORD dwWaitResult=WaitForSingleObject(m_handle_,dwMilliseconds);
    if (dwWaitResult==WAIT_TIMEOUT) SetLastError(ERROR_TIMEOUT);
    assert(dwWaitResult != WAIT_FAILED);
    return dwWaitResult==WAIT_OBJECT_0;
  }
};



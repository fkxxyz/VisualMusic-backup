
// win_event.h

#pragma once
#include "win_object.h"

class CEvent : public CKernelObject {
 public:
  CEvent():m_h_event_(m_handle_) {}
  ~CEvent() { if (m_h_event_) Close(); }

  //CreateEvent
  virtual BOOL Create(
    BOOL bManualReset=TRUE,
    BOOL bInitialState=FALSE,
    LPCTSTR lpName=NULL,
    LPSECURITY_ATTRIBUTES lpEventAttributes=NULL
    )
  {
    assert(m_h_event_==NULL);
    m_h_event_ = CreateEvent(lpEventAttributes, bManualReset,
                             bInitialState, lpName);
#ifdef _DEBUG
    is_seted_ = bInitialState;
#endif
    return m_h_event_ != NULL;
  }

  //OpenEvent
  virtual BOOL Open(LPCTSTR lpName,
      DWORD dwDesiredAccess=EVENT_ALL_ACCESS,BOOL bInheritHandle=FALSE) {
    assert(m_h_event_==NULL);
    m_h_event_ = OpenEvent(dwDesiredAccess,bInheritHandle,lpName);
    return m_h_event_ != NULL;
  }

  virtual BOOL IsSeted() const {
    assert(m_h_event_!=NULL);
    switch (WaitForSingleObject(m_h_event_,0)) {
    case WAIT_OBJECT_0:
#ifdef _DEBUG
      assert(is_seted_ == TRUE);
#endif
      return TRUE;
    case WAIT_TIMEOUT:
#ifdef _DEBUG
      assert(is_seted_ == FALSE);
#endif
      return FALSE;
    default:
      assert(0);
      return FALSE;
    }
  }

  //ResetEvent
  virtual BOOL Reset() {
    assert(m_h_event_!=NULL);
#ifdef _DEBUG
    is_seted_ = FALSE;
#endif
    return ResetEvent(m_h_event_);
  }

  //SetEvent
  virtual BOOL Set() {
    assert(m_h_event_!=NULL);
#ifdef _DEBUG
    is_seted_ = TRUE;
#endif
    return SetEvent(m_h_event_);
  }

  //PulseEvent
  virtual BOOL Pulse() {
    assert(m_h_event_!=NULL);
    return PulseEvent(m_h_event_);
  }

 protected:
  HANDLE &m_h_event_;

#ifdef _DEBUG
  BOOL is_seted_;
#endif
};



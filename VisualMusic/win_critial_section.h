
// WinCritialSection.h

#pragma once

class CCriticalSection {
 public:
  CCriticalSection() { InitializeCriticalSection(&m_critical_section_); }
  ~CCriticalSection() { DeleteCriticalSection(&m_critical_section_); }
  void Enter() { EnterCriticalSection(&m_critical_section_); }
  void Leave() { LeaveCriticalSection(&m_critical_section_); }

#if(_WIN32_WINNT >= 0x0400)
  BOOL TryEnter() { return TryEnterCriticalSection(&m_critical_section_); }
#endif /* _WIN32_WINNT >= 0x0400 */

 private:
  CRITICAL_SECTION m_critical_section_;
};

class CCriticalSectionFunction {
 public:
  CCriticalSectionFunction(CCriticalSection &objCriticalSection)
      : m_objCriticalSection(objCriticalSection) {
    m_objCriticalSection.Enter();
  }
  ~CCriticalSectionFunction() {
    m_objCriticalSection.Leave();
  }

 private:
  CCriticalSection &m_objCriticalSection;
};




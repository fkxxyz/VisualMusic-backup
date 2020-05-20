
// stream_factory.h
#pragma once
#include <cstddef>
#include "cthread.h"

#ifdef WIN32
#ifdef _DEBUG
#define _STREAM_FACTORY_TEST
#endif
#else
#ifdef DEBUG
#define _STREAM_FACTORY_TEST
#endif
#endif

template <class ElemTypeIn, class ElemTypeOut>
class CStreamFactoryTemplate {
 public:
  virtual void Input(ElemTypeIn *data, size_t number) = 0;
  virtual void Output(ElemTypeOut *data, size_t number) = 0;
};


template <class ElemTypeIn, class ElemTypeOut>
class CStreamFactory : protected cth::CThreadProc {
 public:
  CStreamFactory();
  ~CStreamFactory();

  int Start();
  void Reset();
  void Stop();

  int Input(ElemTypeIn *data, size_t number);
  int Output(ElemTypeOut **p_data, size_t *p_number);
  void EndOutput();

 protected:
  virtual void ResetInit() = 0;
  virtual int Working() = 0;  // 返回 1 得到输入，返回 0 得到输出
  virtual void Release() = 0;

  void WaitInput();
  void WaitOutput();

  virtual void ReceiveInput(ElemTypeIn *data, size_t number) = 0;
  virtual void SendOutput(ElemTypeOut **p_data, size_t *p_number) = 0;

 private:

  void BeginInput();
  void EndInput();

  void BeginOutput();

  void *ThreadProc();

  cth::CSemaphore m_semaphore_begin_input_;
  cth::CSemaphore m_semaphore_end_input_;

  cth::CSemaphore m_semaphore_begin_output_;
  cth::CSemaphore m_semaphore_end_output_;

  cth::CSemaphore m_semaphore_operator_lock_;

  int m_working_;
  int m_stop_flag_, m_reset_flag_;
};

template <class ElemTypeIn, class ElemTypeOut>
inline CStreamFactory<ElemTypeIn, ElemTypeOut>::CStreamFactory():
m_semaphore_begin_input_(1, 1),
m_semaphore_end_input_(1, 1),
m_semaphore_begin_output_(1, 1),
m_semaphore_end_output_(1, 1),
m_semaphore_operator_lock_(1, 1)
{
  m_working_ = 0;
  m_semaphore_begin_input_.P();
  m_semaphore_end_input_.P();
  m_semaphore_begin_output_.P();
  m_semaphore_end_output_.P();
}

template <class ElemTypeIn, class ElemTypeOut>
inline CStreamFactory<ElemTypeIn, ElemTypeOut>::~CStreamFactory() {
  Join();
}

template <class ElemTypeIn, class ElemTypeOut>
inline int CStreamFactory<ElemTypeIn, ElemTypeOut>::Start() {
  m_semaphore_operator_lock_.P();
  m_reset_flag_ = 1;
  if (!CThreadProc::Create()) {
    m_semaphore_operator_lock_.V();
    return 0;
  }
  m_semaphore_operator_lock_.P();
  assert(m_reset_flag_ == 0);
  assert(m_stop_flag_ == 0);
  m_semaphore_operator_lock_.V();
  return 1;
}

template <class ElemTypeIn, class ElemTypeOut>
inline void CStreamFactory<ElemTypeIn, ElemTypeOut>::Reset() {
  m_semaphore_operator_lock_.P();
  m_reset_flag_ = 1;
  m_semaphore_operator_lock_.P();
  assert(m_reset_flag_ == 0);
  m_semaphore_operator_lock_.V();
}

template <class ElemTypeIn, class ElemTypeOut>
inline void CStreamFactory<ElemTypeIn, ElemTypeOut>::Stop() {
  m_semaphore_operator_lock_.P();
  m_stop_flag_ = 1;
  m_semaphore_operator_lock_.P();
  assert(m_stop_flag_ == 0);
  m_semaphore_operator_lock_.V();
}

template <class ElemTypeIn, class ElemTypeOut>
inline void *CStreamFactory<ElemTypeIn, ElemTypeOut>::ThreadProc() {
  m_stop_flag_ = 0;
  m_working_ = 1;
  while (!m_stop_flag_) {
    if (m_reset_flag_) {
      ResetInit();
      m_reset_flag_ = 0;
      m_semaphore_operator_lock_.V();
    }
    if (Working())
      WaitInput();
    else
      WaitOutput();
  }
  Release();
  m_stop_flag_ = 0;
  m_working_ = 0;
  WaitOutput();
  m_semaphore_operator_lock_.V();
  return 0;
}

template <class ElemTypeIn, class ElemTypeOut>
inline int CStreamFactory<ElemTypeIn, ElemTypeOut>::Input(ElemTypeIn *data,
                                                          size_t number) {
  if (!m_working_) {
    assert(0);
    return 0;
  }
  m_semaphore_operator_lock_.P();
  BeginInput();
  ReceiveInput(data, number);
  EndInput();
  m_semaphore_operator_lock_.V();
  return 1;
}

template <class ElemTypeIn, class ElemTypeOut>
inline int CStreamFactory<ElemTypeIn, ElemTypeOut>::Output(
    ElemTypeOut **p_data, size_t *p_number) {
  if (!m_working_) {
    assert(0);
    return 0;
  }
  BeginOutput();
  if (!m_working_) {
    EndOutput();
    return 0;
  }
  SendOutput(p_data, p_number);
  return 1;
}


template <class ElemTypeIn, class ElemTypeOut>
inline void CStreamFactory<ElemTypeIn, ElemTypeOut>::WaitInput() {
  m_semaphore_begin_input_.V();
  m_semaphore_end_input_.P();
}

template <class ElemTypeIn, class ElemTypeOut>
inline void CStreamFactory<ElemTypeIn, ElemTypeOut>::WaitOutput() {
  m_semaphore_begin_output_.V();
  m_semaphore_end_output_.P();
}


template <class ElemTypeIn, class ElemTypeOut>
inline void CStreamFactory<ElemTypeIn, ElemTypeOut>::BeginInput() {
  m_semaphore_begin_input_.P();
}

template <class ElemTypeIn, class ElemTypeOut>
inline void CStreamFactory<ElemTypeIn, ElemTypeOut>::EndInput() {
  m_semaphore_end_input_.V();
}


template <class ElemTypeIn, class ElemTypeOut>
inline void CStreamFactory<ElemTypeIn, ElemTypeOut>::BeginOutput() {
  m_semaphore_begin_output_.P();
}

template <class ElemTypeIn, class ElemTypeOut>
inline void CStreamFactory<ElemTypeIn, ElemTypeOut>::EndOutput() {
  m_semaphore_end_output_.V();
}



#ifdef _STREAM_FACTORY_TEST

#include <cassert>
#include <time.h>
#include <stdlib.h>

class CStreamFactoryExampleInt100Sum : public CStreamFactory<int, int> {
 public:
  CStreamFactoryExampleInt100Sum();
  ~CStreamFactoryExampleInt100Sum();

 protected:
  int *m_data;
  size_t m_number;

  int output_sum;
  size_t sum_int_n;

  void ResetInit();
  int Working();
  void Release();
  void ReceiveInput(int *data, size_t number);
  void SendOutput(int **p_data, size_t *p_number);
};

inline CStreamFactoryExampleInt100Sum::~CStreamFactoryExampleInt100Sum() {
  if (m_data) delete m_data;
}

inline CStreamFactoryExampleInt100Sum::CStreamFactoryExampleInt100Sum() {
  m_data = NULL;
}

inline void CStreamFactoryExampleInt100Sum::ResetInit() {
  m_number = 0;
  output_sum = 0;
  sum_int_n = 0;
}

inline int CStreamFactoryExampleInt100Sum::Working() {
  if (m_number == 0)
    return 1;
  for (size_t i = 0; i < m_number; i++) {
    output_sum += m_data[i];
    sum_int_n ++;
    if (sum_int_n == 100) {
      WaitOutput();
      output_sum = 0;
      sum_int_n = 0;
    }
  }
  m_number = 0;
  return 1;
}

inline void CStreamFactoryExampleInt100Sum::Release() {
  Working();
  if (sum_int_n)
    WaitOutput();
}

inline void CStreamFactoryExampleInt100Sum::ReceiveInput(int *data,
                                                         size_t number) {
  if (m_data) delete m_data;
  m_data = new int[number];
  memcpy(m_data, data, number * sizeof(int));
  m_number = number;
}

inline void CStreamFactoryExampleInt100Sum::SendOutput(int **p_data,
                                                       size_t *p_number) {
  *p_data = &output_sum;
  *p_number = 1;
}


class CTP_test_Example_in : public cth::CThreadProc {
  CStreamFactoryExampleInt100Sum &m_s;
  int *m_int_in;
  int m_int_n;

 public:
  CTP_test_Example_in(CStreamFactoryExampleInt100Sum &s,
                      int *int_in, int int_n):m_s(s) {
    srand((int)time(0));
    m_int_in = int_in;
    m_int_n = int_n;
  }

  void *ThreadProc() {
    int pos = 0;
    while (pos != m_int_n) {
      int n = rand();
      if (pos + n > m_int_n)
        n = m_int_n - pos;
      m_s.Input(m_int_in + pos, n);
      pos += n;
    }
    m_s.Stop();
    return 0;
  }
};

class CTP_test_Example_out : public cth::CThreadProc {
  CStreamFactoryExampleInt100Sum &m_s;
  int *m_int_out;
  int m_int_n;

 public:
  CTP_test_Example_out(CStreamFactoryExampleInt100Sum &s,
                       int *int_out, int int_n):m_s(s) {
    m_int_out = int_out;
    m_int_n = int_n;
  }
  void *ThreadProc() {
    int pos = 0;
    int *a;
    size_t n;
    while (m_s.Output(&a, &n)) {
      assert(n == 1);
      assert(*a == m_int_out[pos]);
      pos++;
      m_s.EndOutput();
    }
    assert(pos == m_int_n);
    return 0;
  }
};


const int test_n = 100000;
const int test_out_n = (test_n + 99) / 100;

int test_in[test_n], test_out[test_out_n];

inline int TestStreamFactoryExample() {
  srand((int)time(0));

  int out = 0;
  int out_i = 0;
  int out_n = 0;
  for (int i = 0; i < test_n; i++) {
    test_in[i] = rand();
    out += test_in[i];
    if ((++out_i) == 100) {
      test_out[out_n++] = out;
      out = 0;
      out_i = 0;
    }
  }
  if (out_i) test_out[out_n++] = out;
  assert(out_n == test_out_n);

  CStreamFactoryExampleInt100Sum s;
  s.Start();
  CTP_test_Example_in ctp_in(s, test_in, test_n);
  CTP_test_Example_out ctp_out(s, test_out, test_out_n);
  ctp_in.Create();
  ctp_out.Create();
  ctp_out.Join();
  return 1;
}

#endif


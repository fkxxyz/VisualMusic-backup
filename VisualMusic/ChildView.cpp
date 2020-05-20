// ChildView.cpp : CChildView ���ʵ��
//

#include "stdafx.h"
#include "VisualMusic.h"
#include "ChildView.h"

#include "mfc_double_buffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
  m_player_.Clear();
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
  ON_WM_PAINT()
  ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
  ON_COMMAND(ID_OPERATOR_PLAY, &CChildView::OnOperatorPlay)
  ON_COMMAND(ID_OPERATOR_STOP, &CChildView::OnOperatorStop)
  ON_WM_TIMER()
  ON_WM_ERASEBKGND()
  ON_COMMAND(ID_VIEW_1, &CChildView::OnView1)
  ON_COMMAND(ID_VIEW_2, &CChildView::OnView2)
  ON_COMMAND(ID_VIEW_3, &CChildView::OnView3)
END_MESSAGE_MAP()

// CChildView ��Ϣ�������

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
  if (!CWnd::PreCreateWindow(cs))
    return FALSE;

  cs.dwExStyle |= WS_EX_CLIENTEDGE;
  cs.style &= ~WS_BORDER;
  cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
    ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

  return TRUE;
}

void inline FillFreq(double *a, int time,
                     CAudioPCMAnalyser<double> *analyser, int nSamplesPerSec) {

  const double scale_freq = 1.0293022366434920287823718007739l; // ����Ƶ�ʵı�
  for (int f = 0; f < n_freq / 2; f++) {
    a[f*2] = analyser->GetAmpOfFreq(time, kTone[f].freq);
    a[f*2+1] = analyser->GetAmpOfFreq(time, kTone[f].freq / scale_freq);
  }
}

void CChildView::CalcSpeArray(int time_pos) {

  int iMaxTimePos = m_pcm_.m_num_samples_ /
      (int)((double)m_pcm_.m_num_samples_per_sec_ / 1000);

  static int iLastPos = 0x7FFFFFFF;

  // ����
  for (int t = 0; t < n_time; t++) {
    // �����Ӧ��ʱ��
    int current_time_pos = time_pos + (t - now_n_time) * elem_time;

    // ֱ�����Խ���ʱ���
    if (current_time_pos < 0 || current_time_pos >= iMaxTimePos) {
      for (int f = 0; f < n_freq; f++)
        spe[t][f] = m_pcm_.m_max_amplitude_;
      continue;
    }

    int last_t = t + (time_pos / elem_time - iLastPos / elem_time);
    if (last_t >= 0 && last_t < n_time) {
      // ������һ�μ������õ�ʱ���
      if (iLastPos < time_pos) {
        memcpy(spe + t, spe + last_t, sizeof(spe[0]));
        continue;
      }
    }

    // �ø���Ҷ�任����ʱ���
    FillFreq(spe[t], current_time_pos, &m_analyser_,
             m_pcm_.m_num_samples_per_sec_);
  }

  iLastPos = time_pos;
}

inline void ArrayRectangle(COLORREF *a, int w, COLORREF c,
                           int x1, int y1, int x2, int y2) {
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > w) x2 = w;
  for (int x = x1; x < x2; x++)
    for (int y = y1; y < y2; y++)
      a[x + y * w] = c;
}

COLORREF A[1024*1024*4];

void CChildView::OnPaint() 
{
  CMemPaintDC dc(this); // ���ڻ��Ƶ��豸������

  if (m_player_.GetStatus() >= PCM_PLAYING) {
    register int w = dc.m_rect_.right - dc.m_rect_.left;
    register int h = dc.m_rect_.bottom - dc.m_rect_.top;
    dc.m_bmp_mem_.GetBitmapBits(w*h*4, A);

    switch (m_view) {
    case 1:
      {
        double ff[n_freq];
        FillFreq(ff, m_player_.GetTimePos(),
                 &m_analyser_, m_pcm_.m_num_samples_per_sec_);

        // ���ƺ�ɫ����
        ArrayRectangle(A, w, RGB(0,0,0),
          left,
          top,
          left + n_freq * pix_freq,
          top + n_time * pix_time
          );

        for (int f = 0; f < n_freq; f++) {
          double x = ff[f] / m_pcm_.m_max_amplitude_;
          double y = scale_func(x, sc_k[f]);
          int v = (int)(y * n_time * pix_time);
          ArrayRectangle(A, w, RGB(255,127,0),
            left + (n_freq - f - 1) * pix_freq,
            n_time * pix_time - v,
            left + (n_freq - f) * pix_freq,
            n_time * pix_time
            );
        }
      }
      break;

    case 2:
      {
        double ff[n_freq];
        FillFreq(ff, m_player_.GetTimePos(), &m_analyser_,
                 m_pcm_.m_num_samples_per_sec_);

        // ���ƺ�ɫ����
        ArrayRectangle(A, w, RGB(0,0,0),
          left,
          top,
          left + n_freq * pix_freq,
          top + n_time * pix_time
          );

        for (int f = 0; f < n_freq; f++) {
          double x = ff[f] / m_pcm_.m_max_amplitude_;
          double y = scale_func(x, sc_k[f]);
          int v = (int)(y * n_color);
          if (v > n_color - 1) v = n_color - 1;
          ArrayRectangle(A, w, sp_color[v],
            left + (n_freq - f - 1) * pix_freq,
            top,
            left + (n_freq - f) * pix_freq,
            top + n_time * pix_time
            );
        }
      }
      break;

    case 3:
      {
        // �����������
        CalcSpeArray(m_player_.GetTimePos());


        // ���ƺ�ɫ����
        ArrayRectangle(A, w, RGB(0,0,0),
          left,
          top,
          left + n_freq * pix_freq,
          top + n_time * pix_time
          );

        // �������Ͽ�ʼ����
        for (int t = 0; t < n_time; t++) {
          for (int f = 0; f < n_freq; f++) {
            double x = spe[t][f] / m_pcm_.m_max_amplitude_;
            double y = scale_func(x, sc_k[f]);
            int v = (int)(y * n_color);
            if (v > n_color - 1) v = n_color - 1;
            ArrayRectangle(A, w, sp_color[v],
              left + (n_freq - f - 1) * pix_freq,
              top + (n_time - t - 1) * pix_time,
              left + (n_freq - f) * pix_freq,
              top + (n_time - t) * pix_time
              );
          }
        }

        /*
        // ��ɫ����
        for (int x = 0; x < n_color; x++) {
          ArrayRectangle(A, w, sp_color[x],
            left + x,
            top,
            left + x + 1,
            top + n_time * pix_time
            );
        }
        */

        // ����ʱ��ֱ��
        ArrayRectangle(A, w, RGB(255,255,255),
          left,
          top + (n_time - now_n_time) * pix_time,
          left + n_freq * pix_freq,
          top + (n_time - now_n_time) * pix_time + 1
          );
      }
      break;
    }

    dc.m_bmp_mem_.SetBitmapBits(w*h*4, A);
  }

  dc.Paint();
  // ��ҪΪ������Ϣ������ CWnd::OnPaint()
}

void CChildView::OnFileOpen()
{
  // ����ļ�
  TCHAR szFilters[] = 
    _T("����֧�ֵ���Ƶ�ļ�(*.mp3 *.wav)\0*.mp3;*.wav\0")
    _T("\0")
  ;
  CFileDialog dlgOpen(TRUE);
  dlgOpen.m_ofn.lpstrFilter = szFilters;
  if (dlgOpen.DoModal() != IDOK)
    return;

  m_player_.Clear();
AfxMessageBox(_T("�����ļ�ʱ��������Ϊ��֧�ֵĸ�ʽ��"));
  // ��ȡ�������ļ�
  if (!m_decoder.Decode(dlgOpen.GetPathName().GetBuffer(), &m_pcm_)) {
    AfxMessageBox(_T("�����ļ�ʱ��������Ϊ��֧�ֵĸ�ʽ��"));
    return;
  }

  // ���������� PCM
  m_player_.Load(&m_pcm_);

  // ���������� PCM
  if (!m_player_.Open()) {
    AfxMessageBox(_T("�����ļ�ʱ������ʽ����ȷ��"));
    return;
  }

  AfxGetApp()->GetMainWnd()->SetWindowText(dlgOpen.GetPathName().GetBuffer());

  m_analyser_.Init(&m_pcm_);
  if (0) {
    //m_analyser_.Transform(0);
    //const double c = 1.0293022366434920287823718007739l;

    //const int n = sizeof(kTone)/sizeof(TONE_NAME_FREQ);
    //double rs[n * 2];
    //double f, nv;
    //for (int i = 0; i < n; i++) {
      //if (_tcscmp(_T("A4"), kTone[i].name) == 0)
      //  __asm int 3
      //rs[i * 2] = m_analyser_.Freq(0, kTone[i].freq);
      //TRACE(_T("%lf\t%lf\t%lf\n"), f, nv, rs[i*2]);
      //rs[i * 2 + 1] = m_analyser_.Freq(0, kTone[i].freq / c);
      //TRACE(_T("%lf\t%lf\t%lf\n"), f, nv, rs[i*2+1]);
    //}
    exit(0);
  }

  OnOperatorPlay();
}

void CChildView::OnOperatorPlay()
{
  switch (m_player_.GetStatus()) {
  case PCM_CLOSED:
    break;
  case PCM_READY:
    if (m_view == 3)
      CalcSpeArray(0);
    m_player_.Play();
    SetTimer(1, 1, NULL);
    break;
  case PCM_PLAYING:
    m_player_.Pause();
    KillTimer(1);
    break;
  case PCM_PAUSE:
    m_player_.Resume();
    SetTimer(1, 1, NULL);
    break;
  }
}

void CChildView::OnOperatorStop()
{
  if (m_player_.GetStatus() != PCM_PLAYING)
    return;

  KillTimer(1);
  m_player_.Reset();
  Invalidate();
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
  // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
  Invalidate();
  CWnd::OnTimer(nIDEvent);
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

BOOL CChildView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
                        DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
                        UINT nID, CCreateContext* pContext) {
  for (int i = 0; i < n_color; i++) {
    COLORREF c;
    int x = i % 256;
    switch (i / 256) {
    case 0:
      c = RGB(0, x, 0);
      break;
    case 1:
      c = RGB(0, 255, x);
      break;
    case 2:
      c = RGB(x, 255, 255);
      break;
    }
    sp_color[i] = c;
  }

  m_view = 1;

  for (int i = 0; i < n_freq; i++)
    sc_k[i] = k_func((double)i / n_freq);

  return CWnd::Create(
      lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BOOL CChildView::DestroyWindow()
{

  return CWnd::DestroyWindow();
}

void CChildView::OnView1()
{
  m_view = 1;
  Invalidate();
}

void CChildView::OnView2()
{
  m_view = 2;
  Invalidate();
}

void CChildView::OnView3()
{
  m_view = 3;
  Invalidate();
}


// MFCDoubleBuffer.h
// ˫����� CPaintDC ��

#pragma once

class CMemPaintDC : public CDC {
  CPaintDC m_dc_; //������ͼ���豸������

 public:
  CBitmap m_bmp_mem_; //�ڴ��豸�����ĵ�λͼ
  CRect m_rect_; //��ͼ������

  explicit CMemPaintDC(CWnd* pWnd, DWORD dwRop = WHITENESS)
    :m_dc_(pWnd)
  {
    pWnd->GetClientRect(&m_rect_);
    CDC::CreateCompatibleDC(&m_dc_);
    m_bmp_mem_.CreateCompatibleBitmap(&m_dc_,m_rect_.Width(),m_rect_.Height());
    CDC::SelectObject(&m_bmp_mem_);
    CDC::PatBlt(
      0,
      0,
      m_rect_.right - m_rect_.left,
      m_rect_.bottom - m_rect_.top,
      dwRop
      );
  }
  ~CMemPaintDC() {
    CDC::DeleteDC();
    m_bmp_mem_.DeleteObject();
  }
  BOOL Paint() {
    return m_dc_.BitBlt(0,0,m_rect_.Width(),m_rect_.Height(),this,0,0,SRCCOPY);
  }
  BOOL Paint(int x, int y, int x_src, int y_src) {
    return m_dc_.BitBlt(x,y,m_rect_.Width(),m_rect_.Height(),
        this,x_src,y_src,SRCCOPY);
  }
  BOOL Paint(int x, int y, int n_width, int n_height, int x_src, int y_src) {
    return m_dc_.BitBlt(x,y,n_width,n_height,this,x_src,y_src,SRCCOPY);
  }
};


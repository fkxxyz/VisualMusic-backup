// MainFrm.cpp : CMainFrame ���ʵ��
//

#include "stdafx.h"
#include "VisualMusic.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
  ON_WM_CREATE()
  ON_WM_SETFOCUS()
END_MESSAGE_MAP()

static UINT indicators[] =
{
  ID_SEPARATOR,           // ״̬��ָʾ��
  ID_INDICATOR_CAPS,
  ID_INDICATOR_NUM,
  ID_INDICATOR_SCRL,
};


// CMainFrame ����/����

CMainFrame::CMainFrame()
{
  // TODO: �ڴ���ӳ�Ա��ʼ������
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  // ����һ����ͼ��ռ�ÿ�ܵĹ�����
  if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
    CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
  {
    TRACE0("δ�ܴ�����ͼ����\n");
    return -1;
  }

  /*
  if (!m_wndStatusBar.Create(this) ||
    !m_wndStatusBar.SetIndicators(indicators,
      sizeof(indicators)/sizeof(UINT)))
  {
    TRACE0("δ�ܴ���״̬��\n");
    return -1;      // δ�ܴ���
  }
  */

  RECT rClient, rWindow;
  GetWindowRect(&rWindow);
  GetClientRect(&rClient);
  SetWindowPos(0, 0, 0,
    n_freq * pix_freq +
        ((rWindow.right - rWindow.left)-(rClient.right - rClient.left)),
    n_time * pix_time +
        ((rWindow.bottom - rWindow.top)-(rClient.bottom - rClient.top)),
    SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER
    );

  return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  if( !CFrameWnd::PreCreateWindow(cs) )
    return FALSE;
  // TODO: �ڴ˴�ͨ���޸�
  //  CREATESTRUCT cs ���޸Ĵ��������ʽ

  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  cs.lpszClass = AfxRegisterWndClass(0);
  return TRUE;
}


// CMainFrame ���

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
  CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
  CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame ��Ϣ�������

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
  // ������ǰ�Ƶ���ͼ����
  m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra,
                          AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // ����ͼ��һ�γ��Ը�����
  if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
    return TRUE;

  // ����ִ��Ĭ�ϴ���
  return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}



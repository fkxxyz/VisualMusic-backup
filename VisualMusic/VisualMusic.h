// VisualMusic.h : VisualMusic Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
  #error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CVisualMusicApp:
// �йش����ʵ�֣������ VisualMusic.cpp
//

class CVisualMusicApp : public CWinApp
{
 public:
  CVisualMusicApp();


// ��д
 public:
  virtual BOOL InitInstance();

// ʵ��

 public:
  afx_msg void OnAppAbout();
  DECLARE_MESSAGE_MAP()
};

extern CVisualMusicApp theApp;
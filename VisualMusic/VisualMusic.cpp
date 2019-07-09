// VisualMusic.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "VisualMusic.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVisualMusicApp

BEGIN_MESSAGE_MAP(CVisualMusicApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CVisualMusicApp::OnAppAbout)
END_MESSAGE_MAP()


// CVisualMusicApp ����

CVisualMusicApp::CVisualMusicApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CVisualMusicApp ����

CVisualMusicApp theApp;


// CVisualMusicApp ��ʼ��

#include "AudioPCMAnalyser.h"
#include "AudioDecoder.h"

inline void TRACEA(double *arr, int n){
	for (int i = 0; i < n; i++)
		TRACE(_T("%d\t%lf\n"), i, arr[i]);
	TRACE("\n\n");
}

BOOL CVisualMusicApp::InitInstance()
{
if (0){
	CAudioDecoder decoder;
	CPCMStream<double> pcm;
	decoder.Decode(_T("D:\\Music\\440_20.wav"), &pcm);

	CAudioPCMAnalyser<double> analyser(&pcm);

	const int wave_start = 2794;
	const double f1=415.30469757994513852244178893372l;
	const double f2=466.16376151808991640720312977639;

	analyser.GetAmpOfFreq(wave_start + 20, f2);

	for (int i = -100; i <= 100; i++)
	TRACE(_T("%d, %lf, %lf, %lf\n"),i, 
		analyser.GetAmpOfFreq(wave_start+i, 440),
		analyser.GetAmpOfFreq(wave_start+i, f1),
		analyser.GetAmpOfFreq(wave_start+i, f2)
		);

	return FALSE;
}

	CWinApp::InitInstance();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));
	// ��Ҫ���������ڣ��˴��뽫�����µĿ�ܴ���
	// ����Ȼ��������ΪӦ�ó���������ڶ���
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// ���������ؿ�ܼ�����Դ
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);






	// Ψһ��һ�������ѳ�ʼ���������ʾ����������и���
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// �������к�׺ʱ�ŵ��� DragAcceptFiles
	//  �� SDI Ӧ�ó����У���Ӧ�� ProcessShellCommand  ֮����
	return TRUE;
}


// CVisualMusicApp ��Ϣ�������




// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
void CVisualMusicApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CVisualMusicApp ��Ϣ�������


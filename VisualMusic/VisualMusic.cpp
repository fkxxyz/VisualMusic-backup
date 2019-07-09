// VisualMusic.cpp : 定义应用程序的类行为。
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


// CVisualMusicApp 构造

CVisualMusicApp::CVisualMusicApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CVisualMusicApp 对象

CVisualMusicApp theApp;


// CVisualMusicApp 初始化

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

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	// 若要创建主窗口，此代码将创建新的框架窗口
	// 对象，然后将其设置为应用程序的主窗口对象
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// 创建并加载框架及其资源
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);






	// 唯一的一个窗口已初始化，因此显示它并对其进行更新
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 SDI 应用程序中，这应在 ProcessShellCommand  之后发生
	return TRUE;
}


// CVisualMusicApp 消息处理程序




// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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

// 用于运行对话框的应用程序命令
void CVisualMusicApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CVisualMusicApp 消息处理程序


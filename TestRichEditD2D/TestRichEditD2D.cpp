//
//  TestRichEditD2D.cpp
//  TestRichEditD2D
//
//  created by yu2924 on 2023-05-03
//

#define WINVER		0x0601
#define _WIN32_WINNT	0x0601
#define _WIN32_IE	0x0700
#define _RICHEDIT_VER	0x0500
#include <atlbase.h>
#include <atlapp.h>
extern CAppModule _Module;
#include <atlwin.h>
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlstr.h>

#include "resource.h"

// ================================================================================

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };
	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_RANGE_HANDLER(IDOK, IDCLOSE, OnCmdClose)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());
		return TRUE;
	}
	LRESULT OnCmdClose(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}
};

// ================================================================================
// cf. "RichEditD2D Window Controls" https://devblogs.microsoft.com/math-in-office/richeditd2d-window-controls/

#define EM_SWITCHTOD2D (WM_USER + 389)

class CView : public CWindowImpl<CView, CRichEditCtrl>
{
public:
	//DECLARE_WND_SUPERCLASS(NULL, CRichEditCtrl::GetWndClassName())
	DECLARE_WND_SUPERCLASS(NULL, L"RichEditD2DPT")
	BOOL PreTranslateMessage(MSG*)
	{
		return FALSE;
	}
	BEGIN_MSG_MAP(CView)
	END_MSG_MAP()
};

// ================================================================================

static int numMainFrameInstances = 0;

class CMainFrame
	: public CFrameWindowImpl<CMainFrame>
	, public CUpdateUI<CMainFrame>
	, public CMessageFilter
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
	CView m_view;
	CFont m_font;
	bool mUseD2D;
	CMainFrame(bool used2d) : mUseD2D(used2d)
	{
	}
	virtual BOOL PreTranslateMessage(MSG* msg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(msg)) return TRUE;
		return m_view.PreTranslateMessage(msg);
	}
	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()
	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_VIEW_FONT, OnViewFont)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
	{
		++numMainFrameInstances;
		m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_SAVESEL, WS_EX_CLIENTEDGE);
		RECT rc = {}; GetWindowRect(&rc);
		rc.right = rc.left + 800;
		rc.bottom = rc.top + 400;
		SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOACTIVATE);
		CString title; GetWindowText(title);
		if(mUseD2D)
		{
			m_view.SendMessageW(EM_SWITCHTOD2D, 0, 0);
			SetWindowText(title + L" (D2D)");
		}
		else
		{
			SetWindowText(title + L" (GDI)");
		}
		//m_font = AtlCreateControlFont();
		LOGFONTW lf = {};
		lf.lfHeight = 48;
		lf.lfCharSet = SHIFTJIS_CHARSET;
		lf.lfWeight = FW_NORMAL;
		lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
		lstrcpynW(lf.lfFaceName, L"BIZ UDPゴシック", _countof(lf.lfFaceName));
		m_font.CreateFontIndirectW(&lf);
		m_view.SetFont(m_font);
		m_view.SetWindowTextW(L"美しい日本語テキストレンダリング");
		CMessageLoop* msgLoop = _Module.GetMessageLoop();
		msgLoop->AddMessageFilter(this);
		return 0;
	}
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
	{
		CMessageLoop* msgLoop = _Module.GetMessageLoop();
		msgLoop->RemoveMessageFilter(this);
		--numMainFrameInstances;
		if(numMainFrameInstances <= 0) ::PostQuitMessage(0);
		return 0;
	}
	LRESULT OnFileExit(WORD, WORD, HWND, BOOL&)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}
	LRESULT OnAppAbout(WORD, WORD, HWND, BOOL&)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnViewFont(WORD, WORD, HWND, BOOL&)
	{
		LOGFONTW lf = {}; m_font.GetLogFont(&lf);
		CFontDialog dlg(&lf);
		if(dlg.DoModal())
		{
			if(m_font) m_font.DeleteObject();
			m_font.CreateFontIndirectW(&lf);
			m_view.SetFont(m_font);
		}
		return 0;
	}
};

// ================================================================================

CAppModule _Module;

int Run(LPTSTR cmdline, int cmdshow)
{
	CMessageLoop msgLoop;
	_Module.AddMessageLoop(&msgLoop);
	CMainFrame wndMainGdi(false);
	CMainFrame wndMainD2d(true);
	int r = 0;
	if(wndMainGdi.CreateEx() && wndMainD2d.CreateEx())
	{
		wndMainGdi.ShowWindow(SW_SHOWNA);
		wndMainD2d.ShowWindow(cmdshow);
		r = msgLoop.Run();
	}
	_Module.RemoveMessageLoop();
	return r;
}

int WINAPI _tWinMain(HINSTANCE hinst, HINSTANCE, LPTSTR cmdline, int cmdshow)
{
	::CoInitialize(NULL);
	AtlInitCommonControls(ICC_BAR_CLASSES);
	_Module.Init(NULL, hinst);
//	HMODULE hrec = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
	HMODULE hrec = ::LoadLibraryW(LR"(C:\Program Files\Microsoft Office\root\vfs\ProgramFilesCommonX64\Microsoft Shared\OFFICE16\RICHED20.DLL)");
	int r = Run(cmdline, cmdshow);
	::FreeLibrary(hrec);
	_Module.Term();
	::CoUninitialize();
	return r;
}

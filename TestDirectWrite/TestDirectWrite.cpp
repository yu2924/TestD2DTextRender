//
//  TestDirectWrite.cpp
//  TestDirectWrite
//
//  created by yu2924 on 2023-05-05
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
#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "Dwrite.lib")

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

#define USE_HWNDRENDERTARGET 0

class CView : public CWindowImpl<CView>
{
public:
	DECLARE_WND_CLASS(NULL)
	D2D1_TEXT_ANTIALIAS_MODE d2dTextAntialiasMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
	D2D1_TEXT_ANTIALIAS_MODE getTextAntialiasMode() const
	{
		return d2dTextAntialiasMode;
	}
	void setTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE v)
	{
		d2dTextAntialiasMode = v;
		Invalidate();
	}
	BOOL PreTranslateMessage(MSG*)
	{
		return FALSE;
	}
	BEGIN_MSG_MAP(CView)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()
	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&)
	{
		CPaintDC dc(m_hWnd);
		RECT rc; GetClientRect(&rc);
		// device independents
		CComPtr<ID2D1Factory> d2dFactory;
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);
		CComPtr<IDWriteFactory> dwFactory;
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&dwFactory);
		CComPtr<IDWriteTextFormat> dwTextFormat;
		dwFactory->CreateTextFormat(L"BIZ UDPゴシック", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 48.0f, L"ja-jp", &dwTextFormat);
		dwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		dwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		// device dependents
#if USE_HWNDRENDERTARGET
		CComPtr<ID2D1HwndRenderTarget> d2dRenderTarget;
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
		d2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hWnd, size), &d2dRenderTarget);
#else
		CComPtr<ID2D1DCRenderTarget> d2dRenderTarget;
		D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			0, 0,
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT);
		d2dFactory->CreateDCRenderTarget(&rtp, &d2dRenderTarget);
		d2dRenderTarget->BindDC(dc, &rc);
#endif
		d2dRenderTarget->SetTextAntialiasMode(d2dTextAntialiasMode);
		CComPtr<ID2D1SolidColorBrush> d2dBlackBrush;
		d2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &d2dBlackBrush);
		// render
		d2dRenderTarget->BeginDraw();
		d2dRenderTarget->SetTransform(D2D1::IdentityMatrix());
		d2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
		const WCHAR asz[] = L"美しい日本語テキストレンダリング";
		int lsz = lstrlenW(asz);
		D2D1_RECT_F rcLayout = D2D1::RectF((FLOAT)rc.left, (FLOAT)rc.top, (FLOAT)(rc.right - rc.left), (FLOAT)(rc.bottom - rc.top));
		d2dRenderTarget->DrawText(asz, lsz, dwTextFormat, rcLayout, d2dBlackBrush);
		d2dRenderTarget->EndDraw();
		return 0;
	}
};

// ================================================================================

class CMainFrame
	: public CFrameWindowImpl<CMainFrame>
	, public CUpdateUI<CMainFrame>
	, public CMessageFilter
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
	CView m_view;
	virtual BOOL PreTranslateMessage(MSG* msg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(msg)) return TRUE;
		return m_view.PreTranslateMessage(msg);
	}
	void UIUpdateTaa()
	{
		D2D1_TEXT_ANTIALIAS_MODE taa = m_view.getTextAntialiasMode();
		UISetCheck(ID_TAA_DEFAULT, taa == D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);
		UISetCheck(ID_TAA_CLEARTYPE, taa == D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
		UISetCheck(ID_TAA_GRAYSCALE, taa == D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
		UISetCheck(ID_TAA_ALIASED, taa == D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
	}
	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_TAA_DEFAULT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TAA_CLEARTYPE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TAA_GRAYSCALE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TAA_ALIASED, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()
	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_RANGE_HANDLER(ID_TAA_DEFAULT, ID_TAA_ALIASED, OnTaa)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
	{
		m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
		RECT rc = {}; GetWindowRect(&rc);
		rc.right = rc.left + 800;
		rc.bottom = rc.top + 400;
		SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOACTIVATE);
		CMessageLoop* msgloop = _Module.GetMessageLoop();
		msgloop->AddMessageFilter(this);
		UIUpdateTaa();
		return 0;
	}
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
	{
		CMessageLoop* msgloop = _Module.GetMessageLoop();
		msgloop->RemoveMessageFilter(this);
		::PostQuitMessage(0);
		return 0;
	}
	LRESULT OnFileExit(WORD, WORD, HWND, BOOL&)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}
	LRESULT OnAppAbout(WORD, WORD wID, HWND, BOOL&)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnTaa(WORD, WORD wID, HWND, BOOL&)
	{
		D2D1_TEXT_ANTIALIAS_MODE taa = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
		switch(wID)
		{
			case ID_TAA_DEFAULT: taa = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT; break;
			case ID_TAA_CLEARTYPE: taa = D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE; break;
			case ID_TAA_GRAYSCALE: taa = D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE; break;
			case ID_TAA_ALIASED: taa = D2D1_TEXT_ANTIALIAS_MODE_ALIASED; break;
		}
		m_view.setTextAntialiasMode(taa);
		UIUpdateTaa();
		return 0;
	}
};

// ================================================================================

CAppModule _Module;

int Run(LPTSTR cmdline, int cmdshow)
{
	CMessageLoop msgLoop;
	_Module.AddMessageLoop(&msgLoop);
	CMainFrame wndMain;
	int r = 0;
	if(wndMain.CreateEx())
	{
		wndMain.ShowWindow(cmdshow);
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
	int r = Run(cmdline, cmdshow);
	_Module.Term();
	::CoUninitialize();
	return r;
}

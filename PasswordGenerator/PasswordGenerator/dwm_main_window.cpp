#include "dwm_main_window.h"

#pragma comment (lib, "dwmapi.lib")
#include <dwmapi.h>
#pragma comment (lib, "uxtheme.lib")
#include <uxtheme.h>
#include <vssym32.h>
#include <windowsx.h>


namespace ui
{
	//@cleanup - remove static variables
	static const int KGenBtnId = 100;
	static HICON hIcon{};
	static HICON hIcon_small{};

	static const int TOPEXTENDWIDTH = 60;
	static const int BOTTOMEXTENDWIDTH = 20;
	static const int LEFTEXTENDWIDTH = 8;
	static const int RIGHTEXTENDWIDTH = 8;

	void PaintCustomCaption(HWND hWnd, HDC hdc);
	LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT CustomCaptionProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool* pfCallDWP);
	LRESULT AppWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void DrawRectangle(HDC hdc, int x, int y, int width, int height, COLORREF bg);

	DwmMainWindow::DwmMainWindow(std::string title, HINSTANCE hinst, int posx, int posy, int width, int height) : 
		title_(ConvertToTString(title)), hinst_(hinst), posx_(posx), posy_(posy), width_(width), height_(height)
	{
		HICON hIcon = static_cast<HICON>(LoadImage(hinst,
			MAKEINTRESOURCE(IDI_ICON1),
			IMAGE_ICON,
			32, 32,
			LR_DEFAULTCOLOR));

		HICON hIcon_small = static_cast<HICON>(LoadImage(hinst,
			MAKEINTRESOURCE(IDI_ICON1_SMALL),
			IMAGE_ICON,
			16, 16,
			LR_DEFAULTCOLOR));

		WNDCLASSEX wcex;
		ZeroMemory(&wcex, sizeof(wcex));
		wcex.cbSize = sizeof(wcex);	// WNDCLASSEX size in bytes
		wcex.style = CS_HREDRAW | CS_VREDRAW;		// Window class styles
		wcex.lpszClassName = TEXT("MAIN_WINDOW");	// Window class name
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);	// Window background brush color.
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW); // Window cursor
		wcex.lpfnWndProc = DwmMainWindow::WndProc;
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIcon_small;
		// Register window and ensure registration success.
		if (!RegisterClassEx(&wcex))
			throw std::runtime_error("could not register window: " + title);

		CREATESTRUCT cs;
		ZeroMemory(&cs, sizeof(cs));
		cs.x = posx_;	// Window X position
		cs.y = posy_;	// Window Y position
		cs.cx = width_;	// Window width
		cs.cy = height_;	// Window height
		cs.hInstance = hinst_; // Window instance.
		cs.lpszClass = wcex.lpszClassName;		// Window class name
		cs.lpszName = title_.c_str();	// Window title
		cs.style = WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX;		// Window style
		cs.lpCreateParams = this;

		hwnd_ = CreateWindowEx(
			cs.dwExStyle,
			cs.lpszClass,
			cs.lpszName,
			cs.style,
			cs.x,
			cs.y,
			cs.cx,
			cs.cy,
			cs.hwndParent,
			cs.hMenu,
			cs.hInstance,
			cs.lpCreateParams);

		if (!hwnd_)
		{
			UnregisterClass(title_.c_str(), hinst_);
			throw std::runtime_error("could not create window: " + title);
		}
	}

	DwmMainWindow::~DwmMainWindow()
	{
		UnregisterClass(title_.c_str(), hinst_);
	}

	LRESULT CALLBACK DwmMainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_NCCREATE:
		{
			// set class instance as gwlp_userdata
			CREATESTRUCT * pcs = (CREATESTRUCT*)lparam;
			auto main_window = reinterpret_cast<DwmMainWindow*>(pcs->lpCreateParams);
			SetLastError(0);
			if (SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(main_window)) == 0)
			{
				if (GetLastError() != 0)
					return FALSE;
			}

			return DefWindowProc(hwnd, msg, wparam, lparam);
		}

		default:
		{
			// retrieve class instance to handle messages
			auto main_window = reinterpret_cast<DwmMainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if (main_window)
				return main_window->WndInstProc(hwnd, msg, wparam, lparam);
			else
				return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		}
	}

	LRESULT CALLBACK DwmMainWindow::WndInstProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		bool fCallDWP = true;
		BOOL fDwmEnabled = FALSE;
		LRESULT lRet = 0;
		HRESULT hr = S_OK;

		// Winproc worker for custom frame issues.
		hr = DwmIsCompositionEnabled(&fDwmEnabled);
		if (SUCCEEDED(hr))
		{
			lRet = CustomCaptionProc(hwnd, msg, wparam, lparam, &fCallDWP);
		}

		// Winproc worker for the rest of the application.
		if (fCallDWP)
		{
			lRet = AppWinProc(hwnd, msg, wparam, lparam);
		}
		return lRet;
	}

	//
	// Message handler for handling the custom caption messages.
	//
	LRESULT CustomCaptionProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool* pfCallDWP)
	{
		LRESULT lRet = 0;
		HRESULT hr = S_OK;
		bool fCallDWP = true; // Pass on to DefWindowProc?

		fCallDWP = !DwmDefWindowProc(hWnd, message, wParam, lParam, &lRet);

		// Handle window creation.
		if (message == WM_CREATE)
		{
			RECT rcClient;
			GetWindowRect(hWnd, &rcClient);

			// Inform application of the frame change.
			SetWindowPos(hWnd,
				NULL,
				rcClient.left, rcClient.top,
				rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
				SWP_FRAMECHANGED);

			fCallDWP = true;
			lRet = 0;
		}

		// Handle window activation.
		if (message == WM_ACTIVATE)
		{
			// Extend the frame into the client area.
			MARGINS margins;

			margins.cxLeftWidth = 0;      // 8
			margins.cxRightWidth = 0;    // 8
			margins.cyBottomHeight = 0; // 20
			margins.cyTopHeight = TOPEXTENDWIDTH;       // 27

			hr = DwmExtendFrameIntoClientArea(hWnd, &margins);

			if (!SUCCEEDED(hr))
			{
				// Handle error.
			}

			fCallDWP = true;
			lRet = 0;
		}

		if (message == WM_PAINT)
		{
			HDC hdc;
			{
				PAINTSTRUCT ps;
				hdc = BeginPaint(hWnd, &ps);
				PaintCustomCaption(hWnd, hdc);
				
				EndPaint(hWnd, &ps);
			}

			fCallDWP = true;
			lRet = 0;
		}

		// Handle the non-client size message.
		if ((message == WM_NCCALCSIZE) && (wParam == TRUE))
		{
			// Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
			NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

			pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
			pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
			pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
			pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;

			lRet = 0;

			// No need to pass the message on to the DefWindowProc.
			fCallDWP = false;
		}

		// Handle hit testing in the NCA if not handled by DwmDefWindowProc.
		if ((message == WM_NCHITTEST) && (lRet == 0))
		{
			lRet = HitTestNCA(hWnd, wParam, lParam);

			if (lRet != HTNOWHERE)
			{
				fCallDWP = false;
			}
		}

		*pfCallDWP = fCallDWP;

		return lRet;
	}

	//
	// Message handler for the application.
	//
	LRESULT AppWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		int wmId, wmEvent;
		PAINTSTRUCT ps;
		HDC hdc;
		HRESULT hr;
		LRESULT result = 0;

		switch (message)
		{
		case WM_CREATE:
		{}
		break;
		case WM_COMMAND:
			wmId = LOWORD(wParam);
			wmEvent = HIWORD(wParam);

			// Parse the menu selections:
			switch (wmId)
			{
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			PaintCustomCaption(hWnd, hdc);

			// Add any drawing code here...

			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}


	// Paint the title on the custom frame.
	void PaintCustomCaption(HWND hWnd, HDC hdc)
	{
		RECT rcClient;
		GetClientRect(hWnd, &rcClient);

		HTHEME hTheme = OpenThemeData(NULL, L"CompositedWindow::Window");
		if (hTheme)
		{
			HDC hdcPaint = CreateCompatibleDC(hdc);
			if (hdcPaint)
			{
				int cx = rcClient.right - rcClient.left;
				int cy = rcClient.bottom - rcClient.top;

				// Define the BITMAPINFO structure used to draw text.
				// Note that biHeight is negative. This is done because
				// DrawThemeTextEx() needs the bitmap to be in top-to-bottom
				// order.
				BITMAPINFO dib = { 0 };
				dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				dib.bmiHeader.biWidth = cx;
				dib.bmiHeader.biHeight = -cy;
				dib.bmiHeader.biPlanes = 1;
				dib.bmiHeader.biBitCount = 32;
				dib.bmiHeader.biCompression = BI_RGB;

				HBITMAP hbm = CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
				if (hbm)
				{
					HBITMAP hbmOld = (HBITMAP)SelectObject(hdcPaint, hbm);

					// Setup the theme drawing options.
					DTTOPTS DttOpts = { sizeof(DTTOPTS) };
					DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
					DttOpts.iGlowSize = 15;

					// Select a font.
					LOGFONT lgFont;
					HFONT hFontOld = NULL;
					if (SUCCEEDED(GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
					{
						HFONT hFont = CreateFontIndirect(&lgFont);
						hFontOld = (HFONT)SelectObject(hdcPaint, hFont);
					}

					// Draw the title.
					RECT rcPaint = rcClient;
					rcPaint.top += 8;
					rcPaint.right -= 125;
					rcPaint.left += 8;
					rcPaint.bottom = 50;
					DrawThemeTextEx(hTheme,
						hdcPaint,
						0, 0,
						TEXT("TITLE"),
						-1,
						DT_LEFT | DT_WORD_ELLIPSIS,
						&rcPaint,
						&DttOpts);

					// Blit text to the frame.
					BitBlt(hdc, 0, 0, cx, cy, hdcPaint, 0, 0, SRCCOPY);

					SelectObject(hdcPaint, hbmOld);
					if (hFontOld)
					{
						SelectObject(hdcPaint, hFontOld);
					}
					DeleteObject(hbm);
				}
				DeleteDC(hdcPaint);
			}
			CloseThemeData(hTheme);
		}
	}

	// Hit test the frame for resizing and moving.
	LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam)
	{
		// Get the point coordinates for the hit test.
		POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		// Get the window rectangle.
		RECT rcWindow;
		GetWindowRect(hWnd, &rcWindow);

		// Get the frame rectangle, adjusted for the style without a caption.
		RECT rcFrame = { 0 };
		AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

		// Determine if the hit test is for resizing. Default middle (1,1).
		USHORT uRow = 1;
		USHORT uCol = 1;
		bool fOnResizeBorder = false;

		// Determine if the point is at the top or bottom of the window.
		if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + TOPEXTENDWIDTH)
		{
			fOnResizeBorder = (ptMouse.y < (rcWindow.top - rcFrame.top));
			uRow = 0;
		}
		else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - BOTTOMEXTENDWIDTH)
		{
			uRow = 2;
		}

		// Determine if the point is at the left or right of the window.
		if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + LEFTEXTENDWIDTH)
		{
			uCol = 0; // left side
		}
		else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - RIGHTEXTENDWIDTH)
		{
			uCol = 2; // right side
		}

		// Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
		LRESULT hitTests[3][3] =
		{
			{ HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
		{ HTLEFT,       HTNOWHERE,     HTRIGHT },
		{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
		};

		return hitTests[uRow][uCol];
	}

	void DrawRectangle(HDC hdc, int x, int y, int width, int height, COLORREF bg)
	{
		HPEN pen = CreatePen(PS_NULL, 1, RGB(0, 0, 0));
		HPEN prev_pen = reinterpret_cast<HPEN>(SelectObject(hdc, pen));

		HBRUSH brush = NULL;
		HBRUSH prev_brush = NULL;

		if (bg != 0)
		{
			brush = CreateSolidBrush(bg);
		}
		else
		{
			LOGBRUSH lb;
			ZeroMemory(&lb, sizeof(lb));
			lb.lbStyle = BS_NULL;
			brush = CreateBrushIndirect(&lb);
		}

		prev_brush = reinterpret_cast<HBRUSH>(SelectObject(hdc, brush));

		Rectangle(hdc, x, y, x + width + 1, y + height + 1);

		// Restore previos objects
		SelectObject(hdc, prev_pen);
		SelectObject(hdc, prev_brush);

		// Delete created objects
		DeleteObject(pen);
		DeleteObject(brush);

	}

}
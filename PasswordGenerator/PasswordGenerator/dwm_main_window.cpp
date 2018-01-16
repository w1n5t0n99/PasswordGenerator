#include "dwm_main_window.h"

#pragma comment (lib, "dwmapi.lib")
#include <dwmapi.h>

namespace ui
{
	//@cleanup - remove static variables
	static const int KGenBtnId = 100;
	static HICON hIcon{};
	static HICON hIcon_small{};

	void PaintCustomCaption(HWND hWnd, HDC hdc);

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
		switch (msg)
		{

		case WM_ACTIVATE:
		{
			// Extend the frame into the client area.
			MARGINS margins;

			margins.cxLeftWidth = 8;      // 8
			margins.cxRightWidth = 8;    // 8
			margins.cyBottomHeight = 20; // 20
			margins.cyTopHeight = 27;       // 27

			auto hr = DwmExtendFrameIntoClientArea(hwnd, &margins);

			if (!SUCCEEDED(hr))
			{
				// Handle the error.
			}

			return 0;
		}

		case WM_CREATE:
		{
			RECT rc_client;
			GetWindowRect(hwnd, &rc_client);

			// Inform the application of the frame change.
			SetWindowPos(hwnd,
				NULL,
				rc_client.left,
				rc_client.top,
				rc_client.right - rc_client.left,
				(rc_client.top - rc_client.bottom) * -1,
				SWP_FRAMECHANGED);

			return 0;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hwnd, &ps);
			PaintCustomCaption(hwnd, hdc);
			EndPaint(hwnd, &ps);
			
			return 0;
		}
		
		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

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
				int cx = RECTWIDTH(rcClient);
				int cy = RECTHEIGHT(rcClient);

				// Define the BITMAPINFO structure used to draw text.
				// Note that biHeight is negative. This is done because
				// DrawThemeTextEx() needs the bitmap to be in top-to-bottom
				// order.
				BITMAPINFO dib = { 0 };
				dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				dib.bmiHeader.biWidth = cx;
				dib.bmiHeader.biHeight = -cy;
				dib.bmiHeader.biPlanes = 1;
				dib.bmiHeader.biBitCount = BIT_COUNT;
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
						szTitle,
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


}
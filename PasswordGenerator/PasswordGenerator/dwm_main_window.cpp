#include "dwm_main_window.h"

#include "NanoLog.hpp"

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

		cursor_ = wcex.hCursor;

		CREATESTRUCT cs;
		ZeroMemory(&cs, sizeof(cs));
		cs.x = posx_;	// Window X position
		cs.y = posy_;	// Window Y position
		cs.cx = width_;	// Window width
		cs.cy = height_;	// Window height
		cs.hInstance = hinst_; // Window instance.
		cs.lpszClass = wcex.lpszClassName;		// Window class name
		cs.lpszName = title_.c_str();	// Window title
		cs.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;		// Window style
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

		GetWindowPlacement(hwnd_, &g_wpPrev);

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
		auto res = DefWindowProc(hwnd, msg, wparam, lparam);
		switch (msg)
		{
		case WM_SETCURSOR:
			if (hide_cursor_)
			{
				auto ht = LOWORD(lparam);
				if (ht == HTCLIENT)
				{
					::SetCursor(NULL);
					cursor_active_ = false;
				}
				else
				{
					// @fix - this allows mouse to grab window edges, not sure why!
					if (!cursor_active_)
					{
						::SetCursor(cursor_);
						cursor_active_ = true;
					}
				}
			}
			else
			{
				if (!cursor_active_)
				{
					::SetCursor(cursor_);
					cursor_active_ = true;
				}

			}
			break;
		}

		return res;
	}

	void DwmMainWindow::EnableFullscreen()
	{
		auto dw_style = GetWindowLong(hwnd_, GWL_STYLE);
		if (dw_style & WS_OVERLAPPEDWINDOW)
		{
			MONITORINFO mi = { sizeof(mi) };
			if (GetWindowPlacement(hwnd_, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(hwnd_, MONITOR_DEFAULTTOPRIMARY), &mi))
			{
				SetWindowLong(hwnd_, GWL_STYLE, dw_style & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(hwnd_, HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
			}
		}
	}

	void DwmMainWindow::DisableFullscreen()
	{
		auto dw_style = GetWindowLong(hwnd_, GWL_STYLE);
		if (!(dw_style & WS_OVERLAPPEDWINDOW))
		{
			SetWindowLong(hwnd_, GWL_STYLE, dw_style | WS_OVERLAPPEDWINDOW);
			SetWindowPlacement(hwnd_, &g_wpPrev);
			SetWindowPos(hwnd_, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}

	void DwmMainWindow::Maximize()
	{
		ShowWindow(hwnd_, SW_MAXIMIZE);
	}

	void DwmMainWindow::Minimize()
	{
		ShowWindow(hwnd_, SW_MINIMIZE);
	}

	void DwmMainWindow::SetBorderless()
	{
		auto dw_style = GetWindowLongPtr(hwnd_, GWL_STYLE);
		dw_style &= ~(WS_OVERLAPPEDWINDOW);
		SetWindowLongPtr(hwnd_, GWL_STYLE, dw_style);

		SetWindowPos(hwnd_, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

	}

	void DwmMainWindow::SetBordered()
	{
		auto dw_style = GetWindowLongPtr(hwnd_, GWL_STYLE);
		dw_style |= WS_OVERLAPPEDWINDOW;
		SetWindowLongPtr(hwnd_, GWL_STYLE, dw_style);

		SetWindowPos(hwnd_, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}

	void DwmMainWindow::HideCursor()
	{
		hide_cursor_ = true;
	}

	void DwmMainWindow::ShowCursor()
	{
		hide_cursor_ = false;
	}

}


#include "dwm_main_window.h"

#include "NanoLog.hpp"

#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")

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

	namespace dpi
	{
		class Scaler
		{
		public:
			Scaler() : scale_factor_(0) {};

			void SetScaleFactor(int dpi)
			{
				//windows assumes dpi of 96 for dpi unaware
				scale_factor_ = MulDiv(dpi, 100, 96);
			}

			int GetScaleFactor() { return scale_factor_; }

			int ScaleValue(int value) { return MulDiv(value, scale_factor_, 100); }

			RECT ScaleRect(const RECT& rect)
			{
				RECT res{};
				res.bottom = ScaleValue(rect.bottom);
				res.left = ScaleValue(rect.left);
				res.top = ScaleValue(rect.top);
				res.right = ScaleValue(rect.right);

				return res;
			}

			POINT ScalePoint(const POINT& point)
			{
				POINT res{};
				res.x = ScaleValue(point.x);
				res.y = ScaleValue(point.y);

				return res;
			}

			HFONT CreateScaledFont(HDC hdc, int point_size, tstring font, int font_weight, bool italic, bool underline, bool strikeout, int char_set)
			{
				int scaled_point_size = ScaleValue(point_size);
				//	int lf_height = -MulDiv(scaled_point_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
				HFONT hf = CreateFont(-scaled_point_size, 0, 0, 0, font_weight, italic, underline, strikeout, char_set, 0, 0, 0, 0, font.c_str());
				return hf;
			}
			

		private:
			int scale_factor_;
		};

	}




	DwmMainWindow::DwmMainWindow(std::string title, HINSTANCE hinst, int posx, int posy, int width, int height) :
		title_(ConvertToTString(title)), hinst_(hinst), posx_(posx), posy_(posy), width_(width), height_(height)
	{

		SetHighDpiAware();

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

		dpi::Scaler scaler{};
		UINT     dpiX = 0, dpiY = 0;
		// Get the DPI for the main monitor, and set the scaling factor
		POINT point{};
		point.x = 1;
		point.y = 1;
		auto hMonitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
		auto hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
		scaler.SetScaleFactor(dpiX);
		posx_ = scaler.ScaleValue(posx);
		posy_ = scaler.ScaleValue(posy);
		width_ = scaler.ScaleValue(width);
		height_ = scaler.ScaleValue(height);

		LOG_INFO << "Monitor DPI: " << dpiX << "Scaled Width: " << width_;

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

		GetWindowPlacement(hwnd_, &wp_prev_);

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
		if (msg == WM_LBUTTONDOWN)
		{
			LockCursor();
			HideCursor();
			return 0;
		}
		else if (msg == WM_RBUTTONDOWN)
		{
			FreeCursor();
			ShowCursor();
			return 0;
		}
		else if (msg == WM_KILLFOCUS)
		{
			FreeCursor();
			//@fix - show cursor increments internal counter, probably not the best solution
			ShowCursor();
			return 0;
		}

		return DefWindowProc(hwnd, msg, wparam, lparam);

	}

	void DwmMainWindow::EnableFullscreen()
	{
		auto dw_style = GetWindowLong(hwnd_, GWL_STYLE);
		if (dw_style & WS_OVERLAPPEDWINDOW)
		{
			MONITORINFO mi = { sizeof(mi) };
			if (GetWindowPlacement(hwnd_, &wp_prev_) && GetMonitorInfo(MonitorFromWindow(hwnd_, MONITOR_DEFAULTTOPRIMARY), &mi))
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
			SetWindowPlacement(hwnd_, &wp_prev_);
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
		::ShowCursor(false);
	}

	void DwmMainWindow::ShowCursor()
	{
		::ShowCursor(true);
	}

	void DwmMainWindow::LockCursor()
	{
		RECT rect;
		::GetClientRect(hwnd_, &rect);
		::ClientToScreen(hwnd_, (LPPOINT)& rect);
		::ClientToScreen(hwnd_, (LPPOINT)& rect + 1);
		::ClipCursor(&rect);
	}

	void DwmMainWindow::FreeCursor()
	{
		::ClipCursor(NULL);
	}

	void DwmMainWindow::SetHighDpiAware()
	{
		auto hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		if (FAILED(hr))
			LOG_CRIT << "Setting high DPI awareness failed";
	}

	void DwmMainWindow::SetDpiUnaware()
	{
		auto hr = SetProcessDpiAwareness(PROCESS_DPI_UNAWARE);
		if (FAILED(hr))
			LOG_CRIT << "Setting DPI unaware failed";
	}

}


#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <string>
#include <bitset>
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")

namespace wingui
{

	static const std::bitset<8> KDPIAWARE = 0b00000001;

	class DpiScaler
	{
	public:
		DpiScaler() : scale_factor_(0) {};

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

		HFONT CreateScaledFont(HDC hdc, int point_size, std::string font, int font_weight, bool italic, bool underline, bool strikeout, int char_set)
		{
			int scaled_point_size = ScaleValue(point_size);
			//	int lf_height = -MulDiv(scaled_point_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		//	HFONT hf = CreateFont(-scaled_point_size, 0, 0, 0, font_weight, italic, underline, strikeout, char_set, 0, 0, 0, 0, font.c_str());
		//	return hf;
			return NULL;
		}


	private:
		int scale_factor_;
	};


	class MainWindow
	{
	public:
		MainWindow(std::string title, int posx, int posy, int width, int height, std::bitset<8> wnd_flags);
		~MainWindow();
		
	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		LRESULT CALLBACK WndInstProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		bool SetDpiAwareness();

		int ProcessDpiMessages(MSG msg);


		std::string title_{};
		HINSTANCE hinst_{ NULL };
		HWND hwnd_{ NULL };

		int posx_{};
		int posy_{};
		int width_{};
		int height_{};
		std::bitset<8> wnd_flags_;
		WINDOWPLACEMENT wp_prev_{ sizeof(wp_prev_) };

	};

	LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_NCCREATE:
		{
			// set class instance as gwlp_userdata
			CREATESTRUCT * pcs = (CREATESTRUCT*)lparam;
			auto main_window = reinterpret_cast<MainWindow*>(pcs->lpCreateParams);
			
			//the window must handle dpi awareness in nccreate
			auto res = main_window->SetDpiAwareness();

			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(main_window));
		
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}

		default:
		{
			// retrieve class instance to handle messages
			auto main_window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if (main_window)
				return main_window->WndInstProc(hwnd, msg, wparam, lparam);
			else
				return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		}
	}

	//================================================
	// Set dpi awareness based on flag value
	//================================================
	bool MainWindow::SetDpiAwareness()
	{


		if ((wnd_flags_ & KDPIAWARE).none())
		{
			auto prev_context = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
			return true;
		}
		else
		{
			/* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 is only compatible after Win10 creators update,
			   try DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE if invalid
			*/
			if (auto prev_context = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
			{
				return true;
			}
			else if (auto prev_context = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
			{
				EnableNonClientDpiScaling(hwnd_);
				return true;
			}
			else
			{
				// pre windows 8, windows must handle dpi scaling
				SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
				return false;
			}
		}

		return false;
	}


}
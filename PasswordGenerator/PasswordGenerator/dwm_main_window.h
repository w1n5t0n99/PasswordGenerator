#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "windows_util.h"
#include "resource.h"

namespace ui
{

	class DwmMainWindow
	{
	public:
		DwmMainWindow(std::string title, HINSTANCE hinst, int posx, int posy, int width, int height);
		~DwmMainWindow();

		void EnableFullscreen();
		void DisableFullscreen();

		void Maximize();
		void Minimize();

		void SetBorderless();
		void SetBordered();

		void HideCursor();
		void ShowCursor();

		HWND GetHandle() { return hwnd_; }

	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		LRESULT CALLBACK WndInstProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		tstring title_{};
		HINSTANCE hinst_{NULL};
		HWND hwnd_{ NULL };
		bool tracking_mouse_{ false };

		int posx_{};
		int posy_{};
		int width_{};
		int height_{};		
		WINDOWPLACEMENT g_wpPrev { sizeof(g_wpPrev) };

		HCURSOR cursor_;
		bool hide_cursor_{ false };
		bool cursor_active_{ true };
	};


}
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <string>

#include "windows_util.h"

namespace ui
{

	class MainWindow
	{
	public:
		MainWindow(std::string title, HINSTANCE hinst);
		~MainWindow();

		HWND GetHandle() { return hwnd_; }
		
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndInstProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		tstring title_;
		HINSTANCE hinst_;

		HWND hwnd_{};
		HWND hwnd_btn_{};

		WNDCLASSEX wcex_{};
		CREATESTRUCT cs_{};
	};


}

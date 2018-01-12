#pragma once

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
		
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndInstProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		tstring title_;
		HINSTANCE hinst_;
		HWND hwnd_{};
		WNDCLASSEX wcex_{};
		CREATESTRUCT cs_{};
	};


}

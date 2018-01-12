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
		LRESULT CALLBACK Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		tstring title_;
		HINSTANCE hinst_;
		WNDCLASSEX wcex_{};
		CREATESTRUCT cs_{};
	};


}

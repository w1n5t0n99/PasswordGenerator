#include "main_window.h"
#include "resource.h"

#include "NanoLog.hpp"


namespace ui
{
	static const int IDC_TEST_BTN = 100;
	static HICON hIcon{};
	static HICON hIcon_small{};


	MainWindow::MainWindow(std::string title, HINSTANCE hinst) : hinst_(hinst), title_(ConvertToTString(title))
	{

		auto res = AddFontResourceEx(TEXT("Px437_IBM_PS2thin3.ttf"), FR_PRIVATE, NULL);

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

		ZeroMemory(&wcex_, sizeof(wcex_));
		wcex_.cbSize = sizeof(wcex_);	// WNDCLASSEX size in bytes
		wcex_.style = CS_HREDRAW | CS_VREDRAW;		// Window class styles
		wcex_.lpszClassName = TEXT("MAIN_WINDOW");	// Window class name
		wcex_.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);	// Window background brush color.
		wcex_.hCursor = LoadCursor(hinst, IDC_ARROW); // Window cursor
		wcex_.lpfnWndProc = MainWindow::WndProc;
		wcex_.hIcon = hIcon;
		wcex_.hIconSm = hIcon_small;
		// Register window and ensure registration success.
		if (!RegisterClassEx(&wcex_))
			throw std::runtime_error("could not register window: " + title);

		ZeroMemory(&cs_, sizeof(cs_));
		int window_width = 280;
		int window_height = 240;
		auto screen_width = GetSystemMetrics(SM_CXSCREEN);
		auto screen_height = GetSystemMetrics(SM_CYSCREEN);
		auto pos_x = (screen_width / 2) - (window_width / 2);
		auto pos_y = (screen_height / 2) - (window_height / 2);

		cs_.x = pos_x;	// Window X position
		cs_.y = pos_y;	// Window Y position
		cs_.cx = window_width;	// Window width
		cs_.cy = window_height;	// Window height
		cs_.hInstance = hinst_; // Window instance.
		cs_.lpszClass = wcex_.lpszClassName;		// Window class name
		cs_.lpszName = title_.c_str();	// Window title
		cs_.style = WS_OVERLAPPED | WS_SYSMENU;		// Window style
		cs_.lpCreateParams = this;

		hwnd_ = CreateWindowEx(
			cs_.dwExStyle,
			cs_.lpszClass,
			cs_.lpszName,
			cs_.style,
			cs_.x,
			cs_.y,
			cs_.cx,
			cs_.cy,
			cs_.hwndParent,
			cs_.hMenu,
			cs_.hInstance,
			cs_.lpCreateParams);

		if (!hwnd_)
		{
			UnregisterClass(title_.c_str(), hinst_);
			throw std::runtime_error("could not create window: " + title);
		}

	}

	MainWindow::~MainWindow()
	{
		UnregisterClass(title_.c_str(), hinst_);
	}

	LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCCREATE:
		{
			CREATESTRUCT * pcs = (CREATESTRUCT*)lParam;
			auto main_window = reinterpret_cast<MainWindow*>(pcs->lpCreateParams);
			SetLastError(0);
			if (SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(main_window)) == 0)
			{
				if (GetLastError() != 0)
					return FALSE;
			}

			DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
		}

		default:
		{
			auto main_window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
			if (main_window)
				return main_window->WndInstProc(hWnd, uMsg, wParam, lParam);
			else
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		}
	}

	LRESULT CALLBACK MainWindow::WndInstProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			HDC hdc = GetDC(hWnd);
			auto btn_font = CreateFontUtil(hdc, 14, TEXT("Px437 IBM PS/2thin3"), FW_DONTCARE, false, false, false, 255);
			ReleaseDC(hWnd, hdc);
			int btn_width = 90;
			int btn_height = 36;
			int window_width = 280;
			int window_height = 240;
			hwnd_btn_ = CreateWindow(
				TEXT("BUTTON"), // predefined class name
				TEXT("TEST"), // button text 
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT | BS_CENTER,  // Styles 
				((window_width / 2) - (btn_width / 2)), // x position 
				((window_height / 2) + (btn_height / 2)), // y position 
				btn_width, // width
				btn_height, // height
				hWnd, // parent handle
				(HMENU)IDC_TEST_BTN,
				(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), // module instance
				NULL); // lparam, pointer not needed

			if (!hwnd_btn_)
			{
				UnregisterClass(title_.c_str(), hinst_);
				throw std::runtime_error("could not create window ui element");
			}
		}

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
	
}
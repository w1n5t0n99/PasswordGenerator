#include "main_window.h"
#include "resource.h"

#include "NanoLog.hpp"

#include <windowsx.h>


namespace ui
{
	static const int KGenBtnId = 100;
	static HICON hIcon{};
	static HICON hIcon_small{};

	//======================================
	// main window
	//=======================================

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
		wcex_.hCursor = LoadCursor(NULL, IDC_ARROW); // Window cursor
		wcex_.lpfnWndProc = MainWindow::WndProc;
		wcex_.hIcon = hIcon;
		wcex_.hIconSm = hIcon_small;
		// Register window and ensure registration success.
		if (!RegisterClassEx(&wcex_))
			throw std::runtime_error("could not register window: " + title);

		ZeroMemory(&cs_, sizeof(cs_));
		auto screen_width = GetSystemMetrics(SM_CXSCREEN);
		auto screen_height = GetSystemMetrics(SM_CYSCREEN);
		// set to middle of screen
		posx_ = (screen_width / 2) - (window_width_ / 2);
		posy_ = (screen_height / 2) - (window_height_ / 2);

		cs_.x = posx_;	// Window X position
		cs_.y = posy_;	// Window Y position
		cs_.cx = window_width_;	// Window width
		cs_.cy = window_height_;	// Window height
		cs_.hInstance = hinst_; // Window instance.
		cs_.lpszClass = wcex_.lpszClassName;		// Window class name
		cs_.lpszName = title_.c_str();	// Window title
		cs_.style = WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX;		// Window style
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

		//
		// This prevent the round-rect shape of the overlapped window.
		//
		HRGN rgn = CreateRectRgn(0, 0, window_width_, window_height_);
		SetWindowRgn(hwnd_, rgn, TRUE);

	}

	MainWindow::~MainWindow()
	{
		UnregisterClass(title_.c_str(), hinst_);
		delete gen_btn_;
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

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
			int gen_btn_width = 90;
			int gen_btn_height = 36;
			gen_btn_ = new Button("btn", hWnd, KGenBtnId, ((window_width_ / 2) - (gen_btn_width / 2)), ((window_height_ / 2) + (gen_btn_height / 2)), gen_btn_width, gen_btn_height);
			return 0;
		}

		case WM_COMMAND:
		{
			if (LOWORD(wParam) == KGenBtnId)
			{
				//MessageBox(NULL, TEXT("Button Pressed!"), TEXT("Debug Message"), MB_OK );
				if (active_win_ == 1)
					active_win_ = 0;
				else
					active_win_ = 1;

				SendMessage(hwnd_, WM_NCPAINT, NULL, NULL);
				SendMessage(hwnd_, WM_PAINT, NULL, NULL);
			}
			break;
		}

		case WM_SIZE:
		{
			RECT rect;
			HRGN rgn;
			GetWindowRect(hwnd_, &rect);
			posx_ = rect.left;
			posy_ = rect.top;
			window_width_ = rect.right - rect.left;
			window_height_ = rect.bottom - rect.top;
			//
			// I use this to set a rectangular region for the window, and not a round-rect one.
			//
			rgn = CreateRectRgn(0, 0, rect.right, rect.bottom);
			SetWindowRgn(hwnd_, rgn, TRUE);
			DeleteObject(rgn);
			return 0;
		}

		case WM_NCPAINT:
		{
			HDC hdc = GetWindowDC(hWnd);
			DrawTitlebar(hWnd, hdc);
-			ReleaseDC(hWnd, hdc);
			return 0;
		}

		case WM_PAINT:
		{
			//return DefWindowProc(hWnd, uMsg, wParam, lParam);

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			TextOut(hdc, 20, 20, TEXT("Client Text"), 12);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_NCMOUSEMOVE:
		{
			if (!tracking_mouse_)
			{
				TrackMouse(hWnd);
				tracking_mouse_ = true;
			}

			POINT pt;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			ScreenToClient(hWnd, &pt);
			pt.y *= -1;
			// check caption buttons
			RECT exit_rc;
			SetRect(&exit_rc, window_width_ - exit_btn_width_, 0, window_width_ + 1, titlebar_offset_ + 1);

			int prev_state = exit_btn_state_;
			if (PtInRect(&exit_rc, pt))
			{
				exit_btn_state_ = 2;
				if (prev_state != exit_btn_state_)
				{

					SendMessage(hwnd_, WM_NCPAINT, NULL, NULL);
					SendMessage(hwnd_, WM_PAINT, NULL, NULL);
				}
			}
			else
			{
				exit_btn_state_ = 0;
				if (prev_state != exit_btn_state_)
				{
					SendMessage(hwnd_, WM_NCPAINT, NULL, NULL);
					SendMessage(hwnd_, WM_PAINT, NULL, NULL);
				}
			}		
		return 0;
		}

	
		case WM_MOUSEMOVE:
		{
			if (!tracking_mouse_)
			{
				TrackMouse(hWnd);
				tracking_mouse_ = true;
			}
			return 0;
		}
		
		
		case WM_NCMOUSELEAVE:
		{
			// mouse must not be in non client area
			if (exit_btn_state_ == 2)
			{
				if (active_win_)
					exit_btn_state_ = 0;
				else
					exit_btn_state_ = 1;

				SendMessage(hwnd_, WM_NCPAINT, NULL, NULL);
				SendMessage(hwnd_, WM_PAINT, NULL, NULL);
			}

			tracking_mouse_ = false;

			return 0;
		}

		case WM_MOUSELEAVE:
		{
			// mouse must not be in non client area
			if (exit_btn_state_ == 2)
			{
				if (active_win_)
					exit_btn_state_ = 0;
				else
					exit_btn_state_ = 1;

				SendMessage(hwnd_, WM_NCPAINT, NULL, NULL);
				SendMessage(hwnd_, WM_PAINT, NULL, NULL);
			}

			tracking_mouse_ = false;

			return 0;
		}
		

		case WM_NCACTIVATE:
		{
			if (wParam)
				active_win_ = true;
			else
				active_win_ = false;

			SendMessage(hwnd_, WM_NCPAINT, NULL, NULL);
			SendMessage(hwnd_, WM_PAINT, NULL, NULL);

			return 0;
		}

		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == FALSE)
				ShowWindow(hwnd_, SW_MINIMIZE);
			else
				ShowWindow(hwnd_, SW_SHOW);

			//return DefWindowProc(hWnd, uMsg, wParam, lParam);
			return 0;

		}

		case WM_NCCALCSIZE:
		{
			if (!wParam)
			{
				RECT* rc = reinterpret_cast<RECT*>(lParam);
				RECT rcWindow;
				GetWindowRect(hwnd_, &rcWindow);
				CopyRect(rc, &rcWindow);

				rc->top += titlebar_offset_;
				return 0;
			}
			else
			{
				NCCALCSIZE_PARAMS* ncps = (NCCALCSIZE_PARAMS*)lParam;
				if (ncps) 
				{
					RECT* rc = &ncps->rgrc[0];
					RECT rcDest{};
					RECT rcSrc{};

					rc->top += titlebar_offset_;
				}
				return 0;
			}
		}

		case WM_NCHITTEST:
		{
						
			LRESULT retval = 0;
			RECT rc;
			RECT windowRect;
			POINT pt;

			// Here pt is POINT in screen coordinates
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			// Get window RECT in screen coordinates
			GetWindowRect(hwnd_, &windowRect);
			CopyRect(&rc, &windowRect);

			// Modify window rect to point on caption
			rc.bottom = rc.top + titlebar_offset_;

			// Test if point is inside of caption rect
			if (PtInRect(&rc, pt))
			{
				return HTCAPTION;
			}
		
			return HTCLIENT;
			
		}

		

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}


	}

	void MainWindow::DrawTitlebar(HWND hwnd, HDC hdc)
	{

		DrawRectangle(hdc, 0, 0, window_width_ - exit_btn_width_, titlebar_offset_, bg_color_[active_win_]);

		DrawRectangle(hdc, window_width_ - exit_btn_width_ , 0, exit_btn_width_, titlebar_offset_, exit_color_[exit_btn_state_]);

		HFONT fnt = CreateFontUtil(hdc, 14, TEXT("Px437 IBM PS/2thin3"), FW_DONTCARE, false, false, false, 255);
		HFONT fnt_old = reinterpret_cast<HFONT>(SelectObject(hdc, fnt));

		SetBkMode(hdc, TRANSPARENT);

		TextOut(hdc, 1, 1, title_.c_str(), title_.length());
		TextOut(hdc, window_width_ - 15, 1, TEXT("X"), 2);


		SetBkMode(hdc, OPAQUE);

		SelectObject(hdc, fnt_old);
		DeleteObject(fnt);
	}

	void MainWindow::DrawRectangle(HDC hdc, int x, int y, int width, int height, COLORREF bg)
	{
		HPEN pen = CreatePen(PS_NULL, 1, RGB(0, 0, 0));
		HPEN prev_pen =  reinterpret_cast<HPEN>(SelectObject(hdc, pen));

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

	void MainWindow::TrackMouse(HWND hwnd)
	{
		tme_.cbSize = sizeof(TRACKMOUSEEVENT);
		tme_.dwFlags = TME_LEAVE | TME_NONCLIENT; //Type of events to track & trigger.
		tme_.dwHoverTime = 5000; //How long the mouse has to be in the window to trigger a hover event.
		tme_.hwndTrack = hwnd;
		TrackMouseEvent(&tme_);
	}

	//==================================================
	// Button 
	//====================================================

	Button::Button(std::string text, HWND parent, int id, int posx, int posy, int width, int height) : parent_hwnd_(parent), text_(ConvertToTString(text)), id_(id), posx_(posx),
		posy_(posy), width_(width), height_(height)
	{
		HDC hdc = GetDC(parent_hwnd_);
		auto fnt = CreateFontUtil(hdc, 14, TEXT("Px437 IBM PS/2thin3"), FW_DONTCARE, false, false, false, 255);
		ReleaseDC(parent_hwnd_, hdc);

		hwnd_ = CreateWindow(
			TEXT("BUTTON"), // predefined class name
			text_.c_str(), // button text 
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT | BS_CENTER,  // Styles 
			posx_, // x position 
			posy_, // y position 
			width_, // width
			height_, // height
			parent_hwnd_, // parent handle
			(HMENU)id_,
			(HINSTANCE)GetWindowLongPtr(parent_hwnd_, GWLP_HINSTANCE), // module instance
			NULL); // lparam, pointer 


		//retrieve the previously stored original button window procedure 
		def_proc_= reinterpret_cast<WNDPROC>(static_cast<LONG_PTR>(SetWindowLongPtr(hwnd_, GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(WndProc))));
		SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		SendMessage(hwnd_, WM_SETFONT, (WPARAM)fnt, TRUE);

		DeleteObject(fnt);

		if (!hwnd_)
			throw std::runtime_error("could not create button");
		
	}

	Button::~Button()
	{

	}

	LRESULT CALLBACK Button::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto button = reinterpret_cast<Button*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		return button->WndInstProc(hWnd, uMsg, wParam, lParam);
		
	}

	LRESULT CALLBACK Button::WndInstProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{

		[[fallthrough]];
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case VK_RETURN:
				//Password = GeneratePassword(KMaxPwdLength);
				//InvalidateRect(hwndMain, NULL, TRUE);
				//RedrawWindow(hwndMain, NULL, NULL, RDW_INTERNALPAINT);
				break;
			case VK_TAB:
				SetFocus(parent_hwnd_);
				break;
			case VK_SPACE:
				SendMessage(hwnd_, BM_CLICK, 0, 0);
				break;
			case VK_ESCAPE:
				DestroyWindow(parent_hwnd_);
				break;
			}
		}
		
		default:
			return  def_proc_(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}


}
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <string>
#include <memory>

#include "windows_util.h"

namespace ui
{
	class Button
	{
	public:
		Button(std::string text, HWND parent, int id, int posx, int posy, int width, int height);
		~Button();

	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndInstProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		const tstring text_;
		const int id_;
		WNDPROC def_proc_{ nullptr };
		HWND hwnd_{};
		HWND parent_hwnd_{};

		int width_{};
		int height_{};
		int posx_{};
		int posy_{};
	};


	class MainWindow
	{
	public:
		MainWindow(std::string title, HINSTANCE hinst);
		~MainWindow();

		HWND GetHandle() { return hwnd_; }
		
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndInstProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void DrawTitlebar(HWND hwnd, HDC hdc);
		void DrawRectangle(HDC hdc, int x, int y, int width, int height, COLORREF bg);
		void TrackMouse(HWND hwnd);

	private:
		tstring title_;
		HINSTANCE hinst_;

		HWND hwnd_{};
		Button* gen_btn_{nullptr};

		WNDCLASSEX wcex_{};
		CREATESTRUCT cs_{};

		COLORREF bg_color_[2] = { RGB(140, 140, 140), RGB(100,100,100) };
		COLORREF exit_color_[3] = { RGB(100,100,100), RGB(140, 140, 140), RGB(226, 22, 11) };


		int active_win_ = 1;
		int exit_btn_state_ = 0;
		int window_width_ = 280;
		int window_height_ = 240;
		const int titlebar_offset_ = 20;
		int posx_ = 0;
		int posy_ = 0;

		int exit_btn_x_ = 0;
		int exit_btn_y_ = 0;
		int exit_btn_width_ = 20;
		int exit_btn_height_ = 20;

		TRACKMOUSEEVENT tme_{};
		bool tracking_mouse_ = false;

	};


}

// PasswordGenerator.cpp : Defines the entry point for the application.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
// Header needed for unicode adjustment support
#include <tchar.h>
#pragma warning(disable : 4996)
#include <codecvt>
#include <locale>
#include <sstream>
#include <string>
#include <random>

#include "resource.h"
#include "NanoLog.hpp"

#include "windows_util.h"
#include "simple32.h"
#include "main_window.h"

#define IDC_TEST_BTN (100)
#define IDM_CLEAR_PASSWORD    1100

//using namespace simple32_literals;

namespace
{
	// Globals
	const int KMaxPwdLength = 8;
	const int KMWindowWidth = 280;
	const int KMWindowHeight = 240;
	const int KBtnWidth = 90;
	const int KBtnHeight = 36;
	int ScreenWidth{};
	int ScreenHeight{};
	int MWindowX{};
	int MWindowY{};

	s32::TString Password{};
	HFONT g_hfFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	s32::HFontWrapper KBtnFont = NULL;
	COLORREF g_rgbText = RGB(0, 0, 0);

	HWND hwndButton = NULL;
	HWND hwndMain = NULL;
	WNDPROC KOldProc = (WNDPROC)DefWindowProc;

	HACCEL KHAC = NULL;
	ACCEL KACCELS[1] = { {FCONTROL | FVIRTKEY, 0x43, IDM_CLEAR_PASSWORD } };
}

// function declarations
HFONT SimpleCreateFont(HDC hdc, int point_size, s32::TString font, int font_weight, bool italic, bool underline, bool strikeout, int char_set);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BtnProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EnumFontFamCallback(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID lParam);
s32::TString GeneratePassword(int length);
void DrawPassword(HWND hWnd, int xLeft, int yTop, int xRight, int yBotton, s32::TString password);

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	nanolog::initialize(nanolog::GuaranteedLogger(), " ", "logs", 1);
	Password = TEXT(" ");

	auto res = AddFontResourceEx(TEXT("Px437_IBM_PS2thin3.ttf"), FR_PRIVATE, NULL);

	HICON hIcon = static_cast<HICON>(LoadImage(hInstance,
		MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON,
		32, 32,    // or whatever size icon you want to load
		LR_DEFAULTCOLOR));

	HICON hIcon_small = static_cast<HICON>(LoadImage(hInstance,
		MAKEINTRESOURCE(IDI_ICON1_SMALL),
		IMAGE_ICON,
		16, 16,    // or whatever size icon you want to load
		LR_DEFAULTCOLOR));


	KHAC = CreateAcceleratorTable(KACCELS, 1);

	
		
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));

	wcex.cbSize = sizeof(wcex);	// WNDCLASSEX size in bytes
	wcex.style = CS_HREDRAW | CS_VREDRAW;		// Window class styles
	wcex.lpszClassName = TEXT("CPASSWORDGENERATOR");	// Window class name
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);	// Window background brush color.
	wcex.hCursor = LoadCursor(hInstance, IDC_ARROW); // Window cursor
	wcex.lpfnWndProc = WndProc;
	wcex.hIcon = hIcon;
	wcex.hIconSm = hIcon_small;
	
	// Register window and ensure registration success.
	if (!RegisterClassEx(&wcex))
		return 1;

	// Setup window initialization attributes.
	CREATESTRUCT cs;
	ZeroMemory(&cs, sizeof(cs));

	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	MWindowX = (ScreenWidth / 2) - (KMWindowWidth / 2);
	MWindowY = (ScreenHeight / 2) - (KMWindowHeight / 2);

	cs.x = MWindowX;	// Window X position
	cs.y = MWindowY;	// Window Y position
	cs.cx = KMWindowWidth;	// Window width
	cs.cy = KMWindowHeight;	// Window height
	cs.hInstance = hInstance; // Window instance.
	cs.lpszClass = wcex.lpszClassName;		// Window class name
	cs.lpszName = TEXT("Chris' Password Generator");	// Window title
	cs.style = WS_OVERLAPPED | WS_SYSMENU;		// Window style

										// Create the window.
	hwndMain = ::CreateWindowEx(
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

	// Display the window.
	//::ShowWindow(hwndMain, SW_SHOWDEFAULT);
	//::UpdateWindow(hwndMain);

	ui::MainWindow main_window("Test Window", hInstance);
	ShowWindow(main_window.GetHandle(), SW_SHOWDEFAULT);
	UpdateWindow(main_window.GetHandle());


	MSG msg;
	while (::GetMessage(&msg, main_window.GetHandle(), 0, 0) > 0)
	{
		
		if (!TranslateAccelerator(hwndMain, KHAC, &msg))  // hwnd_button handle to yor button
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	::UnregisterClass(wcex.lpszClassName, hInstance);

	return (int)msg.wParam;
	
}

//============================
// WndProc - Windows Pro
//=============================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		HDC hdc = GetDC(hWnd);
		//KBtnFont = SimpleCreateFont(hdc, 14, TEXT("Times New Roman"), FW_NORMAL, false, false, false);
		KBtnFont = SimpleCreateFont(hdc, 14, TEXT("Px437 IBM PS/2thin3"), FW_DONTCARE, false, false, false, 255);
		int aFontCount[] = { 0, 0, 0 };
		EnumFontFamilies(hdc, (LPCTSTR)NULL,
			(FONTENUMPROC)EnumFontFamCallback, (LPARAM)NULL);
		ReleaseDC(hWnd, hdc);
		hwndButton = CreateWindow(
			TEXT("BUTTON"), // predefined class name
			TEXT("Generate"), // button text 
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT | BS_CENTER,  // Styles 
			((KMWindowWidth / 2) - (KBtnWidth / 2)), // x position 
			KMWindowHeight / 2, // y position 
			KBtnWidth, // width
			KBtnHeight, // height
			hWnd, // parent handle
			(HMENU)IDC_TEST_BTN,
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), // module instance
			NULL); // lparam, pointer not needed

		KOldProc = (WNDPROC)SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (LONG_PTR)BtnProc);
		SendMessage(hwndButton, WM_SETFONT, (WPARAM)KBtnFont.Get(), TRUE);
	}
		break;
	case WM_PAINT:
		DrawPassword(hWnd, 0, 10, KMWindowWidth, KMWindowHeight / 2, Password);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_TEST_BTN)
		{
			Password = GeneratePassword(KMaxPwdLength);
			InvalidateRect(hWnd, NULL, TRUE);
			RedrawWindow(hWnd, NULL, NULL, RDW_INTERNALPAINT);
		}
		else if (LOWORD(wParam) == IDM_CLEAR_PASSWORD)
		{
			Password = TEXT(" ");
			InvalidateRect(hWnd, NULL, TRUE);
			RedrawWindow(hWnd, NULL, NULL, RDW_INTERNALPAINT);
		}
		break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_RETURN:
			Password = GeneratePassword(KMaxPwdLength);
			InvalidateRect(hWnd, NULL, TRUE);
			RedrawWindow(hWnd, NULL, NULL, RDW_INTERNALPAINT);
			break;
		case VK_TAB:
			SetFocus(hwndButton);
			break;
		case VK_ESCAPE:
			DestroyWindow(hwndMain);
			break;
		}
	}
		break;
	default:
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

LRESULT CALLBACK BtnProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_RETURN:
			Password = GeneratePassword(KMaxPwdLength);
			InvalidateRect(hwndMain, NULL, TRUE);
			RedrawWindow(hwndMain, NULL, NULL, RDW_INTERNALPAINT);
			break;
		case VK_TAB:
			SetFocus(hwndMain);
			break;
		case VK_SPACE:
			SendMessage(hwndButton, BM_CLICK, 0, 0);
			break;
		case VK_ESCAPE:
			DestroyWindow(hwndMain);
			break;
		}
	}
		break;
	default:
		return  KOldProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void DrawPassword(HWND hWnd, int xLeft, int yTop, int xRight, int yBotton, s32::TString password)
{
	PAINTSTRUCT    ps;
	RECT rec;
	HDC hdc = BeginPaint(hWnd, &ps);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));
	SetRect(&rec, xLeft, yTop, xRight, yBotton);

	s32::HandleWrapper<HFONT> hf = SimpleCreateFont(hdc, 32, TEXT("Px437 IBM PS/2thin3"), FW_DONTCARE, false, false, false, 255);
	
	if (hf)
	{
		//DeleteObject(g_hfFont);
		//g_hfFont = hf;
		//SelectObject(hdc, hf);
	}
	else
	{
		MessageBox(hWnd, TEXT("Font creation failed!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	HFONT hfOld = (HFONT)SelectObject(hdc, hf.Get());

	DrawText(hdc, password.c_str(), password.length(), &rec, DT_TOP | DT_CENTER);

	SelectObject(hdc, hfOld);
	//DeleteObject(hf);

	EndPaint(hWnd, &ps);
	ReleaseDC(hWnd, hdc);
}

s32::TString GeneratePassword(int length)
{
	const static TCHAR KLookup[36] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	std::random_device rd{};
	std::mt19937_64 re{ rd() };
	std::uniform_int_distribution<int> uniform_dist{ 0, 35 };

	s32::TStringStream ss;

	for (int i = 0; i < length; ++i)
		ss << KLookup[uniform_dist(re)];

	return ss.str();
}

HFONT SimpleCreateFont(HDC hdc, int point_size, s32::TString font, int font_weight, bool italic, bool underline, bool strikeout, int char_set)
{
	HFONT hf;
	int lf_height;
	lf_height = -MulDiv(point_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	hf = CreateFont(lf_height, 0, 0, 0, font_weight, italic, underline, strikeout, char_set, 0, 0, 0, 0, font.c_str());
	return hf;
}

BOOL CALLBACK EnumFontFamCallback(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID lParam)
{
	
	// Log font data
	if (FontType & RASTER_FONTTYPE)
		LOG_INFO << "Font Name: " << ConvertFromTString(lplf->lfFaceName) << " - RASTER - " << static_cast<int>(lplf->lfCharSet);
	else if (FontType & TRUETYPE_FONTTYPE)
		LOG_INFO << "Font Name: " << ConvertFromTString(lplf->lfFaceName) << " - TRUETYPE - " << static_cast<int>(lplf->lfCharSet);
	else
		LOG_INFO << "Font Name: " << ConvertFromTString(lplf->lfFaceName) << " - VECTOR - " << static_cast<int>(lplf->lfCharSet);
		
	return true;

	UNREFERENCED_PARAMETER(lplf);
	UNREFERENCED_PARAMETER(lpntm);
	UNREFERENCED_PARAMETER(lParam);
}
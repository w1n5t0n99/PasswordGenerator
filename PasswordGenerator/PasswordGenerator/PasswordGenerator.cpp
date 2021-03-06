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
#include "dwm_main_window.h"



// function declarations
BOOL CALLBACK EnumFontFamCallback(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID lParam);
s32::TString GeneratePassword(int length);

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	nanolog::initialize(nanolog::GuaranteedLogger(), " ", "logs", 1);

	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	// set to middle of screen
	int width = 300;
	int height  = 250;
	int posx = (screen_width / 2) - (width / 2);
	int posy = (screen_height / 2) - (height / 2);

//	ui::MainWindow main_window("Test Window", hInstance);
	ui::DwmMainWindow main_window("Test Window", hInstance, posx, posy, width, height);
//	ShowWindow(main_window.GetHandle(), SW_SHOWDEFAULT);
	UpdateWindow(main_window.GetHandle());

	main_window.EnableFullscreenBorderless();

	Sleep(1000);

	main_window.DisableFullscreenBorderless();

	MSG msg;
	while (::GetMessage(&msg, main_window.GetHandle(), 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
	
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
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <string>
#include <bitset>
#include <ShellScalingApi.h>
//#pragma comment(lib, "Shcore.lib")

namespace wingui
{

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

	namespace dpi
	{
		enum class SupportedVersion
		{
			Invalid,
			DpiUnaware,
			SystemDpiAware,
			PerMonitorDpiAware,
			PerMonitorDpiAwareV2
		};

		using TDPI_AWARENESS_CONTEXT = HANDLE;
		TDPI_AWARENESS_CONTEXT KDPI_AWARENESS_CONTEXT_UNAWARE = ((DPI_AWARENESS_CONTEXT)-1);
		TDPI_AWARENESS_CONTEXT KDPI_AWARENESS_CONTEXT_SYSTEM_AWARE = ((DPI_AWARENESS_CONTEXT)-2);
		TDPI_AWARENESS_CONTEXT KDPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE = ((DPI_AWARENESS_CONTEXT)-3);

		typedef enum _PROCESS_DPI_AWARENESS
		{
			PROCESS_DPI_UNAWARE = 0,
			PROCESS_SYSTEM_DPI_AWARE = 1,
			PROCESS_PER_MONITOR_DPI_AWARE = 2
		} PROCESS_DPI_AWARENESS;

		typedef enum _DPI_AWARENESS
		{
			DPI_AWARENESS_INVALID = -1,
			DPI_AWARENESS_UNAWARE = 0,
			DPI_AWARENESS_SYSTEM_AWARE = 1,
			DPI_AWARENESS_PER_MONITOR_AWARE = 2
		} DPI_AWARENESS;
		
		// min supported Windows 10, version 1607
		using SetThreadDpiAwarenessContextPtr = std::add_pointer_t<TDPI_AWARENESS_CONTEXT(_In_  DPI_AWARENESS_CONTEXT  dpiContext)>;
		using GetThreadDpiAwarenessContextPtr = std::add_pointer_t<TDPI_AWARENESS_CONTEXT(void)>;
		using GetDpiForWindowPtr = std::add_pointer_t<UINT(_In_ HWND hwnd)>;
		using GetDpiForSystemPtr = std::add_pointer_t<UINT(void)>;
		using EnableNonClientDpiScalingPtr = std::add_pointer_t<BOOL(_In_ HWND hwnd)>;
		using GetAwarenessFromDpiAwarenessContextPtr = std::add_pointer_t<DPI_AWARENESS(_In_ DPI_AWARENESS_CONTEXT value)>;

		SetThreadDpiAwarenessContextPtr SetThreadDpiAwarenessContext;
		GetThreadDpiAwarenessContextPtr GetThreadDpiAwarenessContext;
		GetDpiForWindowPtr GetDpiForWindow;
		GetDpiForSystemPtr GetDpiForSystem;
		EnableNonClientDpiScalingPtr EnableNonClientDpiScaling;
		GetAwarenessFromDpiAwarenessContextPtr GetAwarenessFromDpiAwarenessContext;

		// min supported Windows 8.1
		using SetProcessDpiAwarenessPtr = std::add_pointer_t<HRESULT(_In_ PROCESS_DPI_AWARENESS value)>;
		using GetProcessDpiAwarenessPtr = std::add_pointer_t<HRESULT(_In_ HANDLE hprocess, _Out_ PROCESS_DPI_AWARENESS *value)>;
		using GetDpiForMonitorPtr = std::add_pointer_t<HRESULT(_In_ HMONITOR hmonitor,_In_  MONITOR_DPI_TYPE dpiType, _Out_ UINT *dpiX, _Out_ UINT *dpiY)>;

		SetProcessDpiAwarenessPtr SetProcessDpiAwareness;
		GetProcessDpiAwarenessPtr GetProcessDpiAwareness;
		GetDpiForMonitorPtr GetDpiForMonitor;

		SupportedVersion InitDpiSupport()
		{
			HMODULE shcore = LoadLibraryW(L"Shcore");
			if(!shcore)
				return SupportedVersion::Invalid;

			SetThreadDpiAwarenessContext = reinterpret_cast<decltype(SetThreadDpiAwarenessContext)>(GetProcAddress(shcore, "SetThreadDpiAwarenessContext"));
			GetThreadDpiAwarenessContext = reinterpret_cast<decltype(GetThreadDpiAwarenessContext)>(GetProcAddress(shcore, "GetThreadDpiAwarenessContext"));
			GetDpiForWindow = reinterpret_cast<decltype(GetDpiForWindow)>(GetProcAddress(shcore, "GetDpiForWindow"));
			GetDpiForSystem = reinterpret_cast<decltype(GetDpiForSystem)>(GetProcAddress(shcore, "GetDpiForSystem"));
			EnableNonClientDpiScaling = reinterpret_cast<decltype(EnableNonClientDpiScaling)>(GetProcAddress(shcore, "EnableNonClientDpiScaling"));
			GetAwarenessFromDpiAwarenessContext = reinterpret_cast<decltype(GetAwarenessFromDpiAwarenessContext)>(GetProcAddress(shcore, "GetAwarenessFromDpiAwarenessContext"));

			SetProcessDpiAwareness = reinterpret_cast<decltype(SetProcessDpiAwareness)>(GetProcAddress(shcore, "SetProcessDpiAwareness"));
			GetProcessDpiAwareness = reinterpret_cast<decltype(GetProcessDpiAwareness)>(GetProcAddress(shcore, "GetProcessDpiAwareness"));
			GetDpiForMonitor = reinterpret_cast<decltype(GetDpiForMonitor)>(GetProcAddress(shcore, "GetDpiForMonitor"));

			if (SetThreadDpiAwarenessContext)
				return SupportedVersion::PerMonitorDpiAwareV2;
			else if(SetProcessDpiAwareness)
				return SupportedVersion::PerMonitorDpiAware;
			else
				return SupportedVersion::DpiUnaware;
		}
	}

}
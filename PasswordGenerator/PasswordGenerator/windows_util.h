#pragma once
#include <Windows.h>
// Header needed for unicode adjustment support
#include <codecvt>

#include <string>
#include <sstream>

//==================================================================
// unicode / ascii  
//==================================================================
using tstring = std::basic_string<TCHAR>;
using tstringstream = std::basic_stringstream<TCHAR>;

#ifdef  UNICODE

tstring ConvertToTString(const std::string& src)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(src);
}

std::string ConvertFromTString(const tstring& src)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(src);
}

#else

tstring ConvertToTString(const std::string& src)
{
	return tstring(src);
}

std::string ConvertToTString(const tstring& src)
{
	return tstring(src);
}

#endif // ! UNICODE

//==================================================
// conveniance functions
//==================================================


//==================================================
// RAII wrapper for win32 resources
//===================================================
template <typename Handle>
struct HandleTraits
{
};

template <>
struct HandleTraits<HFONT>
{
	using HandleType = HFONT;

	static constexpr HFONT InvalidValue = NULL;

	static void Close(HFONT handle)
	{
		DeleteObject(handle);
	}
};

template <typename Handle>
class HandleWrapper
{
public:
	HandleWrapper() : handle_(HandleTraits<Handle>::InvalidValue) {}
	HandleWrapper(Handle handle) : handle_{ handle } {}
	~HandleWrapper() { HandleTraits<Handle>::Close(handle_); }

	// owning resource no copies allowed, keep it simple no deep copies implemented
	HandleWrapper(const HandleWrapper<Handle>& other) = delete;
	HandleWrapper<Handle>& operator=(const HandleWrapper<Handle>& other) = delete;
	// moves allowed like unique_ptr
	HandleWrapper(HandleWrapper<Handle>&& other) : handle_{ other.handle_ } { other.handle_ = HandleTraits<Handle>::InvalidValue; }
	HandleWrapper<Handle>& operator=(HandleWrapper<Handle>&& other)
	{
		HandleTraits<Handle>::Close(handle_);
		handle_ = other.handle_;
		other.handle_ = HandleTraits<Handle>::InvalidValue;

		return *this;
	}

	bool operator!() const { return handle_ == HandleTraits<Handle>::InvalidValue; }
	operator bool() const { return handle_ != HandleTraits<Handle>::InvalidValue; }

	Handle& Get() { return handle_; }
	const Handle& Get() const { return handle_; }

private:
	Handle handle_;
};


// helpers
using HFontWrapper = HandleWrapper<HFONT>;

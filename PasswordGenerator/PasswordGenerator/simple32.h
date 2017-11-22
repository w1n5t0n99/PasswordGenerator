#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
// Header needed for unicode adjustment support
#include <tchar.h>

#include <string>
#include <sstream>

namespace s32
{

#ifdef  UNICODE
	using Char = wchar_t;
#else
	using Char = char;
#endif // ! UNICODE

	using TString = std::basic_string<Char>;
	using TStringStream = std::basic_stringstream<Char>;

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
		HandleWrapper(HandleWrapper<Handle>&& other) : handle_{ other.handle_ } { other.handle_ = HandleTraits<Handle>::InvalidValue;}
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
}

namespace simple32_literals
{
#ifdef  UNICODE
	std::wstring operator "" _ts(const char* str, size_t size)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		//std::string narrow = converter.to_bytes(str);
		return converter.from_bytes(str);
	}
#else
	std::string operator "" _ts(const char* str, size_t size)
	{
		return { str };
	}
#endif // ! UNICODE
}

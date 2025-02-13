// Author: Jake Rieger
// Created: 7/30/2024.
//

#pragma once
#pragma warning(disable : 4996)

#include <winsdkver.h>
#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// // DirectX apps don't need GDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <dwrite.h>
#include <d2d1.h>

#include <wrl/client.h>

#include <d3d11_1.h>
#include <dxgi1_6.h>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include <algorithm>
#include <cmath>
#include <codecvt>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <exception>
#include <iterator>
#include <locale>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <tuple>

#ifdef _DEBUG
    #include <dxgidebug.h>
#endif

using Microsoft::WRL::ComPtr;

namespace DX {
    // Helper class for COM exceptions
    class com_exception : public std::exception {
    public:
        com_exception(HRESULT hr) noexcept : result(hr) {}

        const char* what() const noexcept override {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            throw com_exception(hr);
        }
    }
}  // namespace DX

inline void WideToANSI(const std::wstring& value, std::string& converted) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    converted = converter.to_bytes(value);
}

inline void ANSIToWide(const std::string& value, std::wstring& converted) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    converted = converter.from_bytes(value);
}
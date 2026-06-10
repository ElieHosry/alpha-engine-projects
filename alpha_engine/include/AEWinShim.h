/******************************************************************************/
/*!
\file       AEWinShim.h
\brief      Minimal Win32 type/constant shim for the Emscripten (web) build.

\details    The Alpha Engine public headers were written against <windows.h>
            (HWND/HINSTANCE in the API surface, VK_* virtual-key codes in
            AEInput.h). The browser has no <windows.h>, so this header supplies
            just enough opaque types and key-code constants for those headers to
            compile unchanged. It is included ONLY when building with Emscripten;
            the native Windows build still includes the real <windows.h>.
*/
/******************************************************************************/
#ifndef AE_WIN_SHIM_H
#define AE_WIN_SHIM_H

#if defined(__EMSCRIPTEN__)

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdarg>

/* --- Opaque handle / integer types referenced by the public API ---------- */
typedef void*               HWND;
typedef void*               HINSTANCE;
typedef void*               HICON;
typedef void*               HDC;
typedef void*               HGLRC;
typedef void*               HBRUSH;
typedef void*               HMENU;
typedef int                 BOOL;
typedef unsigned int        UINT;
typedef unsigned long       DWORD;
typedef uintptr_t           WPARAM;
typedef intptr_t            LPARAM;
typedef intptr_t            LRESULT;
typedef const char*         LPCSTR;
typedef char*               LPSTR;

#ifndef CALLBACK
#define CALLBACK            /* no calling-convention decoration on the web */
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef MB_OK
#define MB_OK 0
#endif
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (void)(P)   /* silence unused-arg warnings */
#endif

/* --- min/max macros ------------------------------------------------------ */
/* <windows.h> defines these as function-like macros (this project does not
   set NOMINMAX), and game/engine code uses the bare max(a,b)/min(a,b) form.
   Reproduce them on the web so that code compiles identically. */
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

/* --- sprintf_s ----------------------------------------------------------- */
/* Microsoft's secure variant. We reproduce its array-template overload and
   forward to vsnprintf, so existing sprintf_s(buf, "fmt", ...) calls compile
   unchanged on the web. (C++-only; the shim is always included from C++.) */
template <size_t N>
inline int sprintf_s(char (&buffer)[N], const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int result = vsnprintf(buffer, N, format, args);
	va_end(args);
	return result;
}

/* --- Virtual-key codes (standard Win32 values) --------------------------- */
/* AEInput.h maps its AEVK_* constants onto these. Values match Microsoft's
   Virtual-Key Codes so game code that uses AEVK_* keeps the same semantics. */
#define VK_LBUTTON   0x01
#define VK_RBUTTON   0x02
#define VK_MBUTTON   0x04
#define VK_BACK      0x08
#define VK_TAB       0x09
#define VK_RETURN    0x0D
#define VK_SHIFT     0x10
#define VK_CONTROL   0x11
#define VK_MENU      0x12
#define VK_PAUSE     0x13
#define VK_CAPITAL   0x14
#define VK_ESCAPE    0x1B
#define VK_SPACE     0x20
#define VK_PRIOR     0x21
#define VK_NEXT      0x22
#define VK_END       0x23
#define VK_HOME      0x24
#define VK_LEFT      0x25
#define VK_UP        0x26
#define VK_RIGHT     0x27
#define VK_DOWN      0x28
#define VK_SNAPSHOT  0x2C
#define VK_INSERT    0x2D
#define VK_DELETE    0x2E
#define VK_NUMPAD0   0x60
#define VK_NUMPAD1   0x61
#define VK_NUMPAD2   0x62
#define VK_NUMPAD3   0x63
#define VK_NUMPAD4   0x64
#define VK_NUMPAD5   0x65
#define VK_NUMPAD6   0x66
#define VK_NUMPAD7   0x67
#define VK_NUMPAD8   0x68
#define VK_NUMPAD9   0x69
#define VK_MULTIPLY  0x6A
#define VK_ADD       0x6B
#define VK_SUBTRACT  0x6D
#define VK_DECIMAL   0x6E
#define VK_DIVIDE    0x6F
#define VK_F1        0x70
#define VK_F2        0x71
#define VK_F3        0x72
#define VK_F4        0x73
#define VK_F5        0x74
#define VK_F6        0x75
#define VK_F7        0x76
#define VK_F8        0x77
#define VK_F9        0x78
#define VK_F10       0x79
#define VK_F11       0x7A
#define VK_F12       0x7B
#define VK_NUMLOCK   0x90
#define VK_SCROLL    0x91
#define VK_LSHIFT    0xA0
#define VK_RSHIFT    0xA1
#define VK_LCONTROL  0xA2
#define VK_RCONTROL  0xA3
#define VK_LMENU     0xA4
#define VK_RMENU     0xA5
#define VK_OEM_1     0xBA
#define VK_OEM_PLUS  0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2     0xBF
#define VK_OEM_3     0xC0
#define VK_OEM_4     0xDB
#define VK_OEM_5     0xDC
#define VK_OEM_6     0xDD
#define VK_OEM_7     0xDE

#endif /* __EMSCRIPTEN__ */
#endif /* AE_WIN_SHIM_H */

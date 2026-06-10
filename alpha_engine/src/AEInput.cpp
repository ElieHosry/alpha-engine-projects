// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	AEInput.cpp
// Author			:	Sun Tjen Fam, Antoine Abi Chakra
// Creation Date	:	2008/0131
// Purpose			:	Input wrapper
// History			:
// - 2008/01/31		:	- initial implementation
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Includes

#include "AEEngine.h"

#if defined(__EMSCRIPTEN__)
#include <SDL.h>		// web keyboard/mouse state comes from SDL
#endif

// ---------------------------------------------------------------------------
// Defines

#define INPUT_KEY_NUM	256

#define INPUT_WINDOWS			1

// ---------------------------------------------------------------------------
// Globals
extern f32 gResolutionRatioX;
extern f32 gResolutionRatioY;

// ---------------------------------------------------------------------------
// Static variables

#if(INPUT_DIRECT3D == 1)

#define DIRECTINPUT_VERSION 0x0800

#include <D3D9.h>
#include <D3DX9.h>
#include <dinput.h>


static LPDIRECTINPUT8		sDI_Object;
static LPDIRECTINPUTDEVICE8	sDI_KeyboardDevice;

#endif



#if(INPUT_WINDOWS == 1)
	static s32		mCursorPosX, 
					mCursorPosY;

	static s32		mCursorDeltaX, 
					mCursorDeltaY;
#endif

static u8	sKeyCurr[INPUT_KEY_NUM];
static u8	sKeyPrev[INPUT_KEY_NUM];

// TODO: this is very bad
s32      mMouseWheelDeltaPrev; 
s32      mMouseWheelDeltaCurr;

// ---------------------------------------------------------------------------
// Static functions prototypes

// ---------------------------------------------------------------------------
// Functions implementations

AE_API s32 AEInputInit()
{
#if(INPUT_DIRECT3D == 1)

	if(FAILED(DirectInput8Create(ghAESysAppInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&sDI_Object, NULL)))
		return 0;

	if(FAILED(sDI_Object->CreateDevice(GUID_SysKeyboard, &sDI_KeyboardDevice, NULL)))
		return 0;

	if(FAILED(sDI_KeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
		return 0;

	if(FAILED(sDI_KeyboardDevice->SetCooperativeLevel(gAESysWindowHandle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
		return 0;
#endif

	return 1;
}

// ---------------------------------------------------------------------------

AE_API void AEInputReset()
{
	memset(sKeyCurr, 0, sizeof(u8) * INPUT_KEY_NUM);
	memset(sKeyPrev, 0, sizeof(u8) * INPUT_KEY_NUM);

	mCursorPosX = 0; 
	mCursorPosY = 0; 
	mCursorDeltaX = 0; 
	mCursorDeltaY = 0;
	mMouseWheelDeltaPrev = 0;
	mMouseWheelDeltaCurr = 0;
}

// ---------------------------------------------------------------------------

AE_API void AEInputUpdate()
{
#if(INPUT_DIRECT3D == 1)

	BYTE keyState[INPUT_KEY_NUM];

	if (gAESysAppActive)
	{
		sDI_KeyboardDevice->Acquire();
		sDI_KeyboardDevice->GetDeviceState(INPUT_KEY_NUM, (LPVOID)keyState);
	}
	else
	{
		memset(keyState, 0, sizeof(BYTE) * INPUT_KEY_NUM);
	}

	for(u32 i = 0; i < INPUT_KEY_NUM; i++)
	{
		sKeyPrev[i] = sKeyCurr[i];
		sKeyCurr[i] = (keyState[i] & 0x80) ? true : false;
	}

#endif


#if defined(__EMSCRIPTEN__)
	// ----- Web: read keyboard + mouse from SDL, mapped into the VK-indexed array -----
	memcpy(sKeyPrev, sKeyCurr, sizeof(u8) * 256);
	memset(sKeyCurr, 0, sizeof(u8) * 256);

	const Uint8* st = SDL_GetKeyboardState(NULL);

	// Letters A-Z  (VK 0x41.. == SDL_SCANCODE_A..)
	for (int i = 0; i < 26; ++i)
		if (st[SDL_SCANCODE_A + i]) sKeyCurr[0x41 + i] = 0x80;

	// Digits 1-9 then 0
	for (int i = 0; i < 9; ++i)
		if (st[SDL_SCANCODE_1 + i]) sKeyCurr[0x31 + i] = 0x80;
	if (st[SDL_SCANCODE_0]) sKeyCurr[0x30] = 0x80;

	// Function keys F1-F12
	for (int i = 0; i < 12; ++i)
		if (st[SDL_SCANCODE_F1 + i]) sKeyCurr[VK_F1 + i] = 0x80;

	// Named keys
	struct AEKeyMap { unsigned char vk; SDL_Scancode sc; };
	static const AEKeyMap kMap[] = {
		{ VK_ESCAPE, SDL_SCANCODE_ESCAPE }, { VK_SPACE, SDL_SCANCODE_SPACE },
		{ VK_RETURN, SDL_SCANCODE_RETURN }, { VK_TAB, SDL_SCANCODE_TAB },
		{ VK_BACK, SDL_SCANCODE_BACKSPACE }, { VK_DELETE, SDL_SCANCODE_DELETE },
		{ VK_INSERT, SDL_SCANCODE_INSERT }, { VK_HOME, SDL_SCANCODE_HOME },
		{ VK_END, SDL_SCANCODE_END }, { VK_PRIOR, SDL_SCANCODE_PAGEUP },
		{ VK_NEXT, SDL_SCANCODE_PAGEDOWN },
		{ VK_LEFT, SDL_SCANCODE_LEFT }, { VK_RIGHT, SDL_SCANCODE_RIGHT },
		{ VK_UP, SDL_SCANCODE_UP }, { VK_DOWN, SDL_SCANCODE_DOWN },
		{ VK_LSHIFT, SDL_SCANCODE_LSHIFT }, { VK_RSHIFT, SDL_SCANCODE_RSHIFT },
		{ VK_SHIFT, SDL_SCANCODE_LSHIFT },
		{ VK_LCONTROL, SDL_SCANCODE_LCTRL }, { VK_RCONTROL, SDL_SCANCODE_RCTRL },
		{ VK_CONTROL, SDL_SCANCODE_LCTRL },
		{ VK_LMENU, SDL_SCANCODE_LALT }, { VK_RMENU, SDL_SCANCODE_RALT },
		{ VK_MENU, SDL_SCANCODE_LALT },
	};
	for (unsigned i = 0; i < sizeof(kMap) / sizeof(kMap[0]); ++i)
		if (st[kMap[i].sc]) sKeyCurr[kMap[i].vk] = 0x80;

	// Mouse buttons + position
	int mx = 0, my = 0;
	Uint32 mb = SDL_GetMouseState(&mx, &my);
	if (mb & SDL_BUTTON(SDL_BUTTON_LEFT))   sKeyCurr[VK_LBUTTON] = 0x80;
	if (mb & SDL_BUTTON(SDL_BUTTON_RIGHT))  sKeyCurr[VK_RBUTTON] = 0x80;
	if (mb & SDL_BUTTON(SDL_BUTTON_MIDDLE)) sKeyCurr[VK_MBUTTON] = 0x80;

	mCursorDeltaX = mx - mCursorPosX;
	mCursorDeltaY = my - mCursorPosY;
	mCursorPosX = mx;
	mCursorPosY = my;
	mMouseWheelDeltaCurr = mMouseWheelDeltaPrev;
	mMouseWheelDeltaPrev = 0;

#elif(INPUT_WINDOWS == 1)
	// backup the current to previous
	memcpy(sKeyPrev, sKeyCurr, sizeof(u8) * 256);

	// get current keyboard state
	GetKeyboardState(sKeyCurr);

	//if (spSystem)
	{
		POINT coord;

		// get the cursor position and adjust it to the client area
		GetCursorPos	(&coord);
		ScreenToClient	(AESysGetWindowHandle(), &coord);

		mCursorDeltaX	= coord.x - mCursorPosX;
		mCursorDeltaY	= coord.y - mCursorPosY;
		mCursorPosX		= coord.x;
		mCursorPosY		= coord.y;
		mMouseWheelDeltaCurr = mMouseWheelDeltaPrev;
		mMouseWheelDeltaPrev = 0;

		//printf("MouseX = %i, MouseY = %i\n", mCursorPosX, mCursorPosY);
	}
	/*else
	{
		mCursorPosX		= 0;
		mCursorPosY		= 0;
		mCursorDeltaX	= 0;
		mCursorDeltaY	= 0;
	}*/
#endif
}

// ---------------------------------------------------------------------------

AE_API void AEInputExit()
{
#if(INPUT_DIRECT3D == 1)

	if(sDI_KeyboardDevice)
	{
		sDI_KeyboardDevice->Unacquire();
		sDI_KeyboardDevice->Release();
	}

	if(sDI_Object)
	{
		sDI_Object->Release();
	}
#endif
}

// ---------------------------------------------------------------------------

AE_API u8 AEInputCheckCurr(u8 key)
{
	return (sKeyCurr[key] & 0x80) != 0;
}

// ---------------------------------------------------------------------------

AE_API u8 AEInputCheckPrev(u8 key)
{
	return (sKeyPrev[key] & 0x80) != 0;
}

// ---------------------------------------------------------------------------

AE_API u8 AEInputCheckTriggered(u8 key)
{
	return (sKeyCurr[key] & (~sKeyPrev[key]) & 0x80) != 0;
}

// ---------------------------------------------------------------------------

AE_API u8 AEInputCheckReleased(u8 key)
{
	return (sKeyPrev[key] & (~sKeyCurr[key]) & 0x80) != 0;
}


// ---------------------------------------------------------------------------


AE_API void AEInputGetCursorPosition(s32 *pX, s32 *pY)
{
	*pX = (s32)(((f32)mCursorPosX) / gResolutionRatioX);
	*pY = (s32)(((f32)mCursorPosY) / gResolutionRatioY);
}


// ---------------------------------------------------------------------------


AE_API void AEInputGetCursorPositionDelta(s32 *pDeltaX, s32 *pDeltaY)
{
	*pDeltaX = (s32)(((f32)mCursorDeltaX) / gResolutionRatioX);
	*pDeltaY = (s32)(((f32)mCursorDeltaY) / gResolutionRatioX);
}


// ---------------------------------------------------------------------------

AE_API void AEInputShowCursor(s32 Show)
{
	//
	// NOTE(momo): This should work for now.
	// A better implementation is probably to check if 
	// the cursor is showing or not before using ShowCursor
	// but it's possibly premature optimization at that point.
	//
#if defined(__EMSCRIPTEN__)
	SDL_ShowCursor(Show ? SDL_ENABLE : SDL_DISABLE);
#else
	if (Show) {
		while (ShowCursor(1) < 0);
	}
	else {
		while (ShowCursor(0) >= 0);
	}
#endif
}

AE_API void AEInputMouseWheelDelta(s32* pDelta)
{
	(*pDelta) = mMouseWheelDeltaCurr;
}

// ---------------------------------------------------------------------------
// Static functions implementations

// ---------------------------------------------------------------------------




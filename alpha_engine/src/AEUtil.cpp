// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	AEUtil.cpp
// Author			:	Sun Tjen Fam
// Creation Date	:	2008/02/01
// Purpose			:	implementation of the utility library
// History			:
// - 2008/02/01		:	- initial implementation
// - 2010/08/17 : Fixed a number of loss of precision warnings.
// ---------------------------------------------------------------------------

#include "AEExport.h"
#include "AETypes.h"
#include "AEUtil.h"
#if defined(__EMSCRIPTEN__)
  #include <emscripten.h>
#else
  #include <windows.h>
#endif

#define isZero(x) ((x < EPSILON) && (x > -EPSILON))
#define isEqual(x,y) (((x >= y) ? (x-y) : (y-x)) < EPSILON)


// ---------------------------------------------------------------------------
// get the current time in seconds

AE_API f64 AEGetTime(f64* pTime)
{
#if defined(__EMSCRIPTEN__)
	// emscripten_get_now() returns milliseconds since page load.
	f64 r = emscripten_get_now() / 1000.0;

	if (pTime)
		*pTime = r;

	return r;
#else
	s64 f, t;
	f64 r, r0, r1;

	QueryPerformanceFrequency((LARGE_INTEGER*)(&f));
	QueryPerformanceCounter  ((LARGE_INTEGER*)(&t));

  //@FIXED - precision warning
	r0 = f64(t / f);
	r1 = (t - ((t / f) * f)) / (f64)(f);
	r  = r0 + r1;

	if (pTime)
		*pTime = r;//r0 + r1;

	return r;//r0 + r1;
#endif
}

// ---------------------------------------------------------------------------
// return a random number between 0 to 1

AE_API f32 AERandFloat()
{
	return rand() / (f32)(RAND_MAX);
}


//
// Floating point comparison helpers.
//

AE_API s32       AEIsF32Zero(f32 x) {
	return isZero(x);
}

AE_API s32       AEIsF32Equal(f32 a, f32 b) {
	return isEqual(a, b);
}

AE_API s32       AEIsF64Zero(f64 x) {
	return isZero(x); 
}

AE_API s32       AEIsF64Equal(f64 a, f64 b) {
	return isEqual(a, b);
}

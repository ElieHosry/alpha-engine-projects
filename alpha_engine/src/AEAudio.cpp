/******************************************************************************/
/*!
\par        Project: Alpha Engine
\file       AEAudio.h

\author     Gerald Wong
\date       May 5, 2023

\brief      Audio library that wraps around FMOD

\copyright  Copyright (C) 2023 DigiPen Institute of Technology. Reproduction
			or disclosure of this file or its contents without the prior
			written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/

#include "AEExport.h"
#include "AETypes.h"
#include "AEAudio.h"

#if !defined(__EMSCRIPTEN__)
// ===========================================================================
//  Native build: full FMOD-backed implementation (unchanged).
// ===========================================================================

#include "fmod.h"


//
// Audio API
//

#define AE_AUDIO_MAX_CHANNELS				64
#define FMOD_TRUE 1
#define FMOD_FALSE 0


static FMOD_SYSTEM* fmod_system;


AE_API void AEAudioExit(void) {
	if (fmod_system != NULL)
	{
		FMOD_RESULT result = FMOD_System_Release(fmod_system);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
			//printf("audio error");
			//fmod_system = NULL;
			return;
		}

		fmod_system = NULL;
	}
}

AE_API s32 AEAudioInit(void) {
    FMOD_RESULT result = FMOD_System_Create(&fmod_system);
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
		//printf("audio error");
		//fmod_system = NULL;
		return 0;
	}

	result = FMOD_System_Init(fmod_system, AE_AUDIO_MAX_CHANNELS, FMOD_INIT_NORMAL, 0);
	if (result != FMOD_OK) {
		// TODO handle error
		return 0;
	}

	// Create the channel groups (for stopping/pausing and controlling pitch and volume on a per group basis)
#if 0
	for (u32 index = 0; index < AE_AUDIO_INITIAL_SOUND_CAPACITY && result == FMOD_OK; ++index)
	{
		result = FMOD_System_CreateChannelGroup(fmod_system, 0, &channel_groups[index]);
	}
#endif

	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
		AEAudioExit();
		return 0;
	}

	return 1;

}

// According to this: https://documentation.help/FMOD-Ex/FMOD_System_Update.html
// We only need to call this once per game tick. They did not specify whether to do 
// it before or after game logic.
AE_API void AEAudioUpdate(void) {
	FMOD_RESULT result = FMOD_System_Update(fmod_system);
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
	
		// Assume this is a fatal problem and shut down FMOD
		AEAudioExit();
		return;
	}

}


//
// API functions
//

AE_API s32 AEAudioIsValidAudio(AEAudio audio) {
	return audio.fmod_sound != 0;
}


AE_API s32 AEAudioIsValidGroup(AEAudioGroup group) {
	return group.fmod_group != 0;
}


AE_API AEAudioGroup AEAudioCreateGroup() {
	AEAudioGroup ret = { 0 };
	FMOD_System_CreateChannelGroup(fmod_system, 0, &ret.fmod_group);
	return ret;
}

AE_API AEAudio AEAudioLoadSound(const char* filepath) {
	AEAudio audio = { 0 };
	FMOD_System_CreateSound(fmod_system, filepath, FMOD_DEFAULT, NULL, &(audio.fmod_sound));

	return audio;
}

AE_API AEAudio AEAudioLoadMusic(const char* filepath) {
	AEAudio audio = { 0 };
	FMOD_System_CreateStream(fmod_system, filepath, FMOD_DEFAULT, NULL, &(audio.fmod_sound));
	return audio; 
}


AE_API void AEAudioPlay(AEAudio audio, AEAudioGroup group, float volume, float pitch, s32 loops) {
	FMOD_CHANNEL* channel;

	if (AEAudioIsValidAudio(audio) && AEAudioIsValidGroup(group)) {
		FMOD_RESULT result = FMOD_System_PlaySound(fmod_system, audio.fmod_sound, group.fmod_group, FMOD_TRUE, &channel);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
			return;
		}
		FMOD_Channel_SetVolume(channel, volume);
		FMOD_Channel_SetPitch(channel, pitch);
		
		
		if (loops != 0) {
			FMOD_Channel_SetMode(channel, FMOD_LOOP_NORMAL);

			// if it's -1, it will loop infinitely
			FMOD_Channel_SetLoopCount(channel, loops);
		}

		FMOD_Channel_SetPaused(channel, FMOD_FALSE);
	}

}

AE_API void AEAudioResumeGroup(AEAudioGroup group)
{
	if (AEAudioIsValidGroup(group))
	{
		FMOD_RESULT result = FMOD_ChannelGroup_SetPaused(group.fmod_group, FMOD_FALSE);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

AE_API void AEAudioStopGroup(AEAudioGroup group)
{
	if (AEAudioIsValidGroup(group))
	{
		FMOD_RESULT result = FMOD_ChannelGroup_Stop(group.fmod_group);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

AE_API void AEAudioPauseGroup(AEAudioGroup group)
{
	if (AEAudioIsValidGroup(group))
	{
		FMOD_RESULT result = FMOD_ChannelGroup_SetPaused(group.fmod_group, FMOD_TRUE );
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

AE_API void AEAudioSetGroupVolume(AEAudioGroup group, float volume)
{
	if (AEAudioIsValidGroup(group))
	{
		FMOD_RESULT result = FMOD_ChannelGroup_SetVolume(group.fmod_group, volume);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}


AE_API void AEAudioSetGroupPitch(AEAudioGroup group, float pitch)
{
	if (AEAudioIsValidGroup(group))
	{
		FMOD_RESULT result = FMOD_ChannelGroup_SetPitch(group.fmod_group, pitch);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

AE_API void AEAudioUnloadAudio(AEAudio audio) {
	FMOD_RESULT result = FMOD_Sound_Release(audio.fmod_sound);
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
	}
}

AE_API void AEAudioUnloadAudioGroup(AEAudioGroup group) {
	FMOD_RESULT result = FMOD_ChannelGroup_Release(group.fmod_group);
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
	}
}

#else
// ===========================================================================
//  Web build: FMOD has no Emscripten backend yet, so the audio API is stubbed
//  out as no-ops. The game links and runs silently; a web audio backend
//  (e.g. SDL_mixer or OpenAL) is a later decision. Signatures match AEAudio.h.
// ===========================================================================

AE_API s32  AEAudioInit(void)   { return 1; }
AE_API void AEAudioUpdate(void) {}
AE_API void AEAudioExit(void)   {}

AE_API s32  AEAudioIsValidAudio(AEAudio audio)        { (void)audio; return 0; }
AE_API s32  AEAudioIsValidGroup(AEAudioGroup group)   { (void)group; return 0; }

AE_API AEAudioGroup AEAudioCreateGroup(void)          { AEAudioGroup g = { 0 }; return g; }
AE_API AEAudio AEAudioLoadSound(const char* filepath) { (void)filepath; AEAudio a = { 0 }; return a; }
AE_API AEAudio AEAudioLoadMusic(const char* filepath) { (void)filepath; AEAudio a = { 0 }; return a; }

AE_API void AEAudioPlay(AEAudio audio, AEAudioGroup group, float volume, float pitch, s32 loops)
                                                      { (void)audio; (void)group; (void)volume; (void)pitch; (void)loops; }
AE_API void AEAudioResumeGroup(AEAudioGroup group)    { (void)group; }
AE_API void AEAudioStopGroup(AEAudioGroup group)      { (void)group; }
AE_API void AEAudioPauseGroup(AEAudioGroup group)     { (void)group; }
AE_API void AEAudioSetGroupVolume(AEAudioGroup group, float volume) { (void)group; (void)volume; }
AE_API void AEAudioSetGroupPitch(AEAudioGroup group, float pitch)   { (void)group; (void)pitch; }
AE_API void AEAudioUnloadAudio(AEAudio audio)         { (void)audio; }
AE_API void AEAudioUnloadAudioGroup(AEAudioGroup group) { (void)group; }

#endif // !__EMSCRIPTEN__

# Playing Audio

## Differences between Sound and Music
Alpha Engine supports loading of multiple types of audio files including `.mp3` and `.wav` formats. 

In Alpha Engine, audio is separated into 2 types: `Sound` and `Music`. 

`Sounds` are audios that are fully loaded into the memory before being played. 
They are most suitable to short sound effects that are played repeatedly. 
To load a Sound-type audio, simply use this function:

```c
// Loads a sound from given filepath and assign to 'audio'
AEAudio audio = AEAudioLoadSound("filepath");
```

`Music` are audios that are streamed from file. 
This means that they are loaded as separate chunks into the memory as it is being played. 
For example, the first 10 seconds of a music is first loaded into memory.
Before the 10 seconds is done playing, the next 10 seconds is loaded. 
This repeats until the music is done. 


To load a Music-type audio, simply use this function:

```c
// Loads a music from given filepath and assign it to 'audio'
AEAudio audio = AEAudioLoadMusic("filepath");
```

One noticable difference between `Sound` and `Music` is that multiple `Sound` from the same  `AEAudio` can overlap each other when you play them (up to a certain amount, of course). 
Multiple `Music` from the same `AEAudio`, however, cannot overlap each other. 

That is, if you use `AEAudioPlay` on multiple `Sounds` from the same `AEAudio` at once, you will hear multiple `Sounds` being played. 
If you use `AEAudioPlay` on multiple `Music` at once, only the last `Music` will be played; the previous 



## Playing an Audio 
`AEAudio` is a type that represents an audio (which can either be a `Sound` or `Music`) loaded by Alpha Engine. 
Before we can play an audio, we must first create an audio group for the audio to play in. 

You can create an audio group simply with the following function:

```c
// Creates an audio group
AEAudioGroup audio_group = AEAudioCreateGroup();
```

`AEAudioGroup` is a type that represents an audio group where `AEAudios` are played in. 
Typically, we would create one `AEAudioGroup` for background music and another audio group for sound effects. 

To play an audio, we simply use `AEAudioPlay`. 

As an example, we will load a sound from a file named "ore.mp3" in the Assets folder and play it at the start of the application.
Write the code below before entering the game loop:


```c
// Loads a sound from a file named 'ore.mp3' in the 'Assets' folder 
// and assign it to 'ore'
AEAudio ore = AEAudioLoadSound("Assets/ore.mp3");

// Creates an audio group named 'sound_effect'
AEAudioGroup sound_effect = AEAudioCreateGroup();

// Plays 'ore' audio in the "sound_effect" audio group with 
// 100% volume, 100% pitch, looped infinitely.
AEAudioPlay(ore, sound_effect, 1.f, 1.f, -1);
```

Upon compiling and running the application, you should be able to hear a sound.

Next, we will load a background music to play. 
This time, we will lower the volume and increase the pitch of the music.
Note that we are now using `AEAudioLoadMusic` instead!
Write the code below before entering the game loop:

```c
// Loads a sound from a file named 'bouken.mp3' in the 'Assets' folder 
// and assign it to 'bouken'.
AEAudio bouken = AEAudioLoadMusic("Assets/bouken.mp3");

// Creates an audio group named 'bgm'
AEAudioGroup bgm = AEAudioCreateGroup();

// Plays 'bouken' audio in the "bgm" audio group with 
// 50% volume, 200% pitch, looped infinitely.
AEAudioPlay(bouken, bgm, 0.5f, 2.f, -1);
```

When you run the application this time, you should be able to hear a soft music playing in the background in a very fast pace. 
Try to play "bouken.mp3" in your preferred music player and compare the differences!

## Releasing resources
Finally, we will need to clean up the AEAudioGroups and AEAudios that we created.
`AEUnloadAudio` will free the resources used by a `AEAudio` object.
`AEUnloadAudioGroup` will free the resources used by a `AEAudioGroup` object.
Thus, before exiting the application, we will write the following code to clean our audio up:

```c
// Release our audios.
AEUnloadAudio(bouken);
AEUnloadAudio(ore)

// Release our audio groups.
AEUnloadAudioGroup(sound_effect);
AEUnloadAudioGroup(bgm);
```

## Sample code
Sample code regarding audio can be found in `snippets\playing_audio.cpp`.
For more information related to audio, check out the documentation surrounding the `AEAudio.h` file.

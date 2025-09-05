#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

#include <fmod.h>
#include <fmod_errors.h>

//#define MUSIC_ON

FMUSIC_MODULE *mod = NULL;
FSOUND_SAMPLE *song = 0;
FSOUND_DSPUNIT    *DrySFXUnit = NULL;

void SoundInit()
{
#ifdef MUSIC_ON
    if (FSOUND_GetVersion() < FMOD_VERSION)
    {
        printf("Error : You are using the wrong DLL version!  You should be using FMOD %.02f\n", FMOD_VERSION);
        exit(1);
    }

    if (!FSOUND_Init(32000, 64, 0))
    {
        printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
        exit(1);
    }
#endif
}

void PlaySong()
{
#ifdef MUSIC_ON
    mod = FMUSIC_LoadSong("sl-star3.xm");
    if (!mod)
    {
        printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
        exit(1);
    }
    FMUSIC_PlaySong(mod);
#endif
}


/*
void PlayMP3()
{
#ifdef MUSIC_ON
	song = FSOUND_Sample_Load(FSOUND_FREE, "push.mp3", FSOUND_2D, 0, 0);

    if (!song)
    {
        printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
        exit(1);
    }
	FSOUND_PlaySoundEx(FSOUND_FREE, song, DrySFXUnit, FALSE);
#endif
}
*/


void SoundEnd()
{
#ifdef MUSIC_ON
    FMUSIC_FreeSong(mod);
	//FSOUND_Sample_Free(song);
    FSOUND_Close();
#endif
}

#define MUSIC_ON

#ifdef MUSIC_ON
#include <stdio.h>
#include <stdlib.h>

#include "mikmod.h"

#ifdef _WIN32
#include <windows.h>

static DWORD WINAPI upd_thread(void *cls);

#else
#include <pthread.h>
#include <unistd.h>

static void *update(void *cls);
#endif


MODULE *mikmod;


void SoundInit()
{
	MikMod_RegisterDriver(&drv_sdl);
	MikMod_RegisterDriver(&drv_nos);

	MikMod_RegisterLoader(&load_xm);

	if(MikMod_Init((char*)"")) {
		fprintf(stderr, "failed to initialize mikmod: %s\n", MikMod_strerror(MikMod_errno));
		abort();
	}
	MikMod_InitThreads();

	{
#ifdef _WIN32
		HANDLE thr;
		if((thr = CreateThread(0, 0, upd_thread, 0, 0, 0))) {
			CloseHandle(thr);
		}
#else
		pthread_t upd_thread;
		if(pthread_create(&upd_thread, 0, update, 0) == 0) {
			pthread_detach(upd_thread);
		}
#endif
	}

}

void PlaySong()
{
	if(!(mikmod = Player_Load((char*)"Data/sl-star3.xm", 128, 0))) {
        fprintf(stderr, "failed to load module: sl-star3.xm: %s\n", MikMod_strerror(MikMod_errno));
		abort();
    }
	Player_Start(mikmod);
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
	Player_Stop();
	MikMod_Exit();
}

#ifdef _WIN32
static DWORD WINAPI upd_thread(void *cls)
#else
static void *update(void *cls)
#endif
{
	for(;;) {
		if(Player_Active()) {
			MikMod_Update();
		}
#ifdef _WIN32
		Sleep(10);
#else
		usleep(10000);
#endif
	}
	return 0;
}

#else	/* !defined MUSIC_ON */

void SoundInit()
{
}

void PlaySong()
{
}

void SoundEnd()
{
}

#endif	/* MUSIC_ON */

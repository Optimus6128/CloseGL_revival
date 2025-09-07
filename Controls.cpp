#include <windows.h>
#include <SDL/SDL.h>

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glaux.h> 

#include "Script.h"

extern int npart;
extern int parts;

extern float rx,ry,rz,tz;
extern int mmo;

bool showFps = false;
bool blend;
bool wire;
bool controls;
bool keys[512];

int partime;

//extern int way,nways;
extern int tex,ntexs;

// ========= Events =========

void KeyCommands()
{
SDL_Event event;
SDL_keysym* keysym;

	while (SDL_PollEvent(&event))
	{
		keysym=&event.key.keysym;
		switch (event.type) 
		{
		case SDL_QUIT:
			quit_demo(0);
			break;

		case SDL_KEYDOWN:

			switch (keysym->sym)
			{

			case SDLK_ESCAPE:
				quit_demo(0);
				break;

			case SDLK_SPACE:
				//npart=(npart+1) % parts;
				//partime=SDL_GetTicks();
			break;


			case SDLK_w:
				//wire=!wire;
				break;

			case SDLK_f:
				showFps = !showFps;
			break;

			case SDLK_t:
				//tex=(tex+1) % ntexs;
			break;

			case SDLK_a:
				//blend=!blend;
			break;

			case SDLK_RETURN:
				 //controls=!controls;
			break;

			default:
				keys[keysym->sym]=true;
				break;
			}
		break;

		case SDL_KEYUP:
			keys[keysym->sym]=false;
			break;

			default:
				break;

		}
	}
}

void KeyEffects()
{

	if (blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	float test=128.0f/(float)mmo;


	if (keys[SDLK_UP] || keys[SDLK_KP8])
		rx+=test;

	if (keys[SDLK_DOWN] || keys[SDLK_KP2])
		rx-=test;

	if (keys[SDLK_LEFT] || keys[SDLK_KP4])
		ry-=test;

	if (keys[SDLK_RIGHT] || keys[SDLK_KP6])
		ry+=test;

	if (keys[SDLK_COMMA] || keys[SDLK_KP7])
		rz+=test;

	if (keys[SDLK_PERIOD] || keys[SDLK_KP9])
		rz-=test;
		
	if (keys[SDLK_KP_PLUS])
		tz-=test;

	if (keys[SDLK_KP_MINUS])
		tz+=test;

}
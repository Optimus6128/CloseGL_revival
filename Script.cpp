#pragma warning(disable: 4244)
#pragma warning(disable: 4305)

#include <windows.h>
#include <math.h>
#include <SDL/SDL.h>
#include <gl/gl.h>

#include "Parts.h"
#include "controls.h"
#include "Sound.h"
#include "GridCalcs.h"
#include "VertexSend.h"


const int ntex=9;

extern GLuint texture[ntex];
extern bool showFps;

int npart=5, parts=6;

float rx=0,ry=0,rz=0;
float tx=0,ty=0,tz=0;
bool apath2=true;

int fps=0,mo=0,kk=0,mmo=0;
Uint32 atime=0;

extern int k;
extern bool controls;
extern bool blend;

bool eba=false;
bool splash1=false;
float r2dx=3.0f, r2dy=0.0f;
float duckx,ducky,duckz,ducktx=r2dx,duckty=r2dy,ducktz,ducktz0,ducktzspeed0=0.0f;
float gravity, grv=-0.025f; float ducktzspeed=0.0f; float waterplane=16.0f;
extern int partime;
extern float duckrpx, duckrpy;
extern bool ripple;
int rippletime;
float skat=1.0f;

int globalTime;

	float ttx,tty,ttz;

void quit_demo( int code )
{
	SoundEnd();
    SDL_Quit();
    exit(code);
}



void TextScript()
{
	int prt=globalTime-partime;
	glBindTexture(GL_TEXTURE_2D, texture[4]);

	float fdintime=4096+216;

	if (prt<fdintime)
	{
		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_COLOR);
		VS_Fade1((float)prt/(fdintime/256.0f),-64.0f);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glEnable(GL_TEXTURE_2D);
	}

	if (prt<8192+fdintime)
	{
		ttx=sin(prt/256.0f)*((prt-16384-fdintime)/1024.0f)-16.0f; tty=cos(prt/256.0f)*((prt-16384-fdintime)/1024.0f)+4.0f; ttz=-(32*(prt-fdintime)/2048.0f);
		glColor3ub(255+ttz*2, 255+ttz*2, 255+ttz*2);
		VS_TextWrite("Nasty Bugs"+0,ttx,tty,ttz);
	}

	if (prt>8191+fdintime && prt<16384+fdintime)
	{
		ttx=sin(prt/256.0f)*((prt-24336-fdintime)/1024.0f)-16.0f; tty=cos(prt/256.0f)*((prt-24336-fdintime)/1024.0f)+4.0f; ttz=-(32*(prt-8192-fdintime)/2048.0f);
		glColor3ub(255+ttz*2, 255+ttz*2, 255+ttz*2);
		VS_TextWrite("presents!"+0,ttx,tty,ttz);
	}

	if (prt>16384+fdintime && prt<16384+fdintime+fdintime)
	{
		glDisable(GL_TEXTURE_2D);
		VS_Fade2((prt-16384-fdintime)/(fdintime/256.0f),-64.0f);
		glEnable(GL_TEXTURE_2D);
	}

	if (prt>16384+fdintime+fdintime)
	{
		glDisable(GL_TEXTURE_2D);
		VS_Fade2(255,-64.0f);
		glEnable(GL_TEXTURE_2D);
		npart=1;
		partime=SDL_GetTicks();
	}
}

void DuckScript()
{
	int prt=globalTime-partime;

	if (prt<4000)
	{
		duckx=0.0f; ducky=0.0f; duckz=0.0f;
		ducktx=0.0f+r2dx; duckty=0.0f+r2dy; ducktz0=76.0f, ducktz=ducktz0;
	}

	if (prt>=4000 && prt<4600)
	{
		ducktz=ducktz0 + ducktzspeed*((float)(prt-4000)/12.0f);
		ducktzspeed=ducktzspeed0 + gravity*((float)(prt-4000)/12.0f);

		if (ducktz<waterplane)
		{
			gravity=-grv*4.0f;
		}
		else
		{
			gravity=grv;
		}

	}

	if (prt>=4600 && prt<=12800)
	{
		splash1=true;

		ducktz=waterplane+cos(globalTime/256.0f)*2.0f/skat - 6.0f;
		duckx=sin(globalTime/256.0f)*16.0f/skat;
		ducky=sin(globalTime/192.0f)*8.0f/skat;
		skat+=0.005*(mmo/64.0f);
	}

	if (prt>=12800)
	{

		if (eba)
		{
		duckx=0.0f; ducky=0.0f;
		ducktx= duckrpx-(float)wqx/2.0f+r2dx;
		duckty= duckrpy-(float)wqy/2.0f+r2dy;
		}
		else
		{
			ducktz=10.0f;
			ripple=true;
			rippletime=globalTime;
			eba=true;
		}
	}
}


void RunPart()
{
	switch(npart)
	{

		case 0:

			if (controls==false)
			{
				rx=0; ry=0;	rz=0;
				tz=-160;
			}

			k=(float)globalTime/24.0f;
			blend=false;
			P_Distort();

		break;


		case 1:

			k=(float)globalTime/32.0f;
			blend=true;

			if (controls==false)
			{
				rx=-45.0f; ry=0.0f;	rz=(float)globalTime/32.0f; tz=-120.0f;
			}

			P_Plasma();


		break;


		case 2:

			if (controls==false)
			{
				int prt=globalTime-partime;
				rx=(float)globalTime/16.0f; ry=(float)globalTime/12.0f; rz=(float)globalTime/28.0f;
				tz=-160+sin(prt/(24.0f * 45.0f))*96.0f;
			}

			k=(float)globalTime/24.0f;
			blend=false;
			P_Polar();

		break;



		case 3:

			if (apath2)
			{
				rx=-60.0f; ry=0.0f; rz=0.0f;
				tz=-120.0f;
			}

			blend=false;
			P_Water();
			DuckScript();

		break;


		case 4:

			if (controls==false)
			{
				rx=-90.0f; ry=45.0f; rz=0.0f;
				tz=-120.0f;
			}

			blend=false;
			P_Spherical();

		break;

		case 5:

			if (controls==false)
			{
				rx=0; ry=0; rz=0;
				tz=0;
			}

			P_Stars();
			TextScript();
		break;

		default:
			break;
	}

}



void FpsCount()
{
	static char fpsStr[16];

	if (globalTime-atime>=1000)
		{
		atime = SDL_GetTicks();
		mmo=((kk-fps)+mo)>>1;
		mo=(kk-fps);
		fps=kk;

		_itoa(mo, fpsStr, 10);
		if (showFps) {
			SDL_WM_SetCaption(fpsStr, fpsStr);
		}
	}
	if (mmo<20) mmo=20;
	if (mmo>400) mmo=400;
}

void RunScript()
{
	globalTime = SDL_GetTicks();

	FpsCount();
    KeyCommands();
	KeyEffects();
	RunPart();
	SDL_GL_SwapBuffers();
	kk++;
}

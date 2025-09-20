#include <windows.h>
#include <SDL/SDL.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "glext.h"
#include <gl/glaux.h>

#include "TextureLoad.h"
#include "precalcs.h"
#include "Script.h"
#include "Sound.h"

int width;
int height;
float ratio;
bool windowed;

bool pointSpritesSupported;

extern int partime;

int k;

PFNGLPOINTPARAMETERFEXTPROC			glPointParameterf			=NULL;
PFNGLPOINTPARAMETERFVARBPROC		glPointParameterfv			=NULL;

// ========= Setup =========

static void setup_opengl()
{

	glPointParameterf  =	(PFNGLPOINTPARAMETERFEXTPROC)wglGetProcAddress("glPointParameterf");
	glPointParameterfv =	(PFNGLPOINTPARAMETERFVEXTPROC)wglGetProcAddress("glPointParameterfv");

	// Maybe I could try to use this? But breaks on AMD
	pointSpritesSupported = (glPointParameterf && glPointParameterfv);

	pointSpritesSupported = false; // Turn it off! Slower in old GPUs (probably CPU emulated?)
	// Since I am trying to optimize demo for Pentium 2 with old GeForce4MX, f*** point sprites and not well supported extensions!

	LoadGLTextures();

	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glColor4f(1.0f,1.0f,1.0f,1.0f);

    glShadeModel(GL_SMOOTH);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, ratio, 1.0f, 4096.0f);

	if (pointSpritesSupported) {
		glPointParameterf(GL_POINT_SIZE_MIN_EXT, 1.0);
		glPointParameterf(GL_POINT_SIZE_MAX_EXT, 256.0);
		float quadratic[] = {0.0f, 0.0f, 0.000001f};
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
		glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
	}
}



void ReadCFG()
{
	FILE *cfg;
	char ca;
	char *wdt="WIDTH=";
	char *hgt="HEIGHT=";
	char *fls="FULLSCREEN=";

	char psitri[256];
	char *psistart=psitri;

	cfg=fopen("CloseGL.cfg","rb");


	for (int j=0; j<3; j++)
	{
		int i;

		ca=fgetc(cfg);

		if (ca==(*wdt))
		{
			do{
				ca=fgetc(cfg);
			}while (ca!=61);

			i=0;
			ca=fgetc(cfg);
			do{
				psitri[i++]=ca;
				ca=fgetc(cfg);
			}while (ca!=10);

			width=atoi(psitri);
		}

		if (ca==(*hgt))
		{
			do{
				ca=fgetc(cfg);
			}while (ca!=61);

			i=0;
			ca=fgetc(cfg);
			do{
				psitri[i++]=ca;
				ca=fgetc(cfg);
			}while (ca!=10);

			height=atoi(psitri);
		}


		if (ca==(*fls))
		{
			do{
				ca=fgetc(cfg);
			}while (ca!=61);


			i=0;
			ca=fgetc(cfg);
			do{
				psitri[i++]=ca;
				ca=fgetc(cfg);
			}while (!EOF);

			i=0;
			do{
				ca=psitri[i++];
			}while (ca<65 || ca>91);

			if (ca=='T') windowed=false;
			if (ca=='F') windowed=true;
		}
	}

	fclose(cfg);
}


// =-=-= Main =-=-=

int main( int argc, char* argv[] )
{

	ReadCFG();
	ratio = (float)width / (float) height;

    const SDL_VideoInfo* info = NULL;
    int bpp = 0;
    int flags = 0;

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        fprintf( stderr, "Video initialization failed: %s\n",
            SDL_GetError( ) );
			quit_demo(1);
    }

    info = SDL_GetVideoInfo( );

    if( !info ) {
        fprintf( stderr, "Video query failed: %s\n",
            SDL_GetError( ) );
			quit_demo(1);
    }

	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption("CloseGL", "CloseGL");

    bpp = info->vfmt->BitsPerPixel;

    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    flags = SDL_OPENGL;
	if (!windowed) flags = flags | SDL_FULLSCREEN;

    if( SDL_SetVideoMode( width, height, bpp, flags ) == 0 ) {
        fprintf( stderr, "Video mode set failed: %s\n",
            SDL_GetError( ) );
			quit_demo(1);
    }

    setup_opengl();
	Precalculations();
	LoadObject2();

	SoundInit();
	PlaySong();

	partime=SDL_GetTicks();

    while( 1 ) {
		RunScript();
    }

    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	return main(nCmdShow, (char**)lpCmdLine);
}

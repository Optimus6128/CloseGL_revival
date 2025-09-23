#include <windows.h>
#include <SDL.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "glext.h"
#include <gl/glaux.h>

#include "TextureLoad.h"
#include "precalcs.h"
#include "Script.h"
#include "Sound.h"

const int width = 800;
const int height = 600;
const float ratio = (float)width / (float) height;

extern int partime;

int i,j,k,l;
int x,y,z,c;

PFNGLPOINTPARAMETERFEXTPROC			glPointParameterf			=NULL;
PFNGLPOINTPARAMETERFVARBPROC		glPointParameterfv			=NULL;

// ========= Setup =========

static void setup_opengl()
{



glPointParameterf  =	(PFNGLPOINTPARAMETERFEXTPROC)wglGetProcAddress("glPointParameterf");
glPointParameterfv =	(PFNGLPOINTPARAMETERFVEXTPROC)wglGetProcAddress("glPointParameterfv");


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

	glPointParameterf(GL_POINT_SIZE_MIN_EXT, 1.0);
	glPointParameterf(GL_POINT_SIZE_MAX_EXT, 256.0);
	float quadratic[] = {0.0f, 0.0f, 0.000001f};
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
	glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );


}



// =-=-= Main =-=-=

int main( int argc, char* argv[] )
{

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

    bpp = info->vfmt->BitsPerPixel;

    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    flags = SDL_OPENGL | SDL_FULLSCREEN;

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
	//PlayMP3();

	partime=SDL_GetTicks();

    while( 1 ) {
		RunScript();
    }

    return 0;
}

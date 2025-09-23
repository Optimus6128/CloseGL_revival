#pragma warning(disable: 4244)
#pragma warning(disable: 4305)

#include <windows.h>
#include <math.h>
#include <SDL.h>

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glaux.h>
#include "glext.h"

#include "GridCalcs.h"
#include "VertexSend.h"
#include "precalcs.h"
#include "Parts.h"


GLfloat LightAmbient[]= { 0.5f, 0.5f, 1.0f, 1.0f };
GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[]= { 0.0f, 0.0f, 2.0f, 1.0f };

extern bool side[6];
extern GLuint texture[1];
extern bool controls;

extern float rx,ry,rz;
extern float tx,ty,tz;
float apath1;
float blockblend[6][6];

float fontx[7], fonty[7];
float fx0[7], fy0[7];
float fx1[7], fy1[7];
float fdx[7], fdy[7];

extern int mmo;

extern int k;
extern float duckx,ducky,duckz,ducktx,duckty,ducktz;
int kl=0;
int wft;

extern unsigned char rcol[nblob],gcol[nblob],bcol[nblob];

extern char fconv[256];
extern int partime;

	float efkol;
	float amg=64.0f;
extern int npart;


star3d star[1024];
flower shape[32];

float yspd=0.0f, yacl=0.01f;

void P_Distort()
{

	GC_Distort();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	VS_Distort();
}


void P_Plasma()
{

	int prt=SDL_GetTicks()-partime;

	if (prt<22528.0f) glClearColor(0.5f, 0.0f, 0.25f, 0.0f);
	GC_Plasma();

	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();


	if (prt>=4096.0f && prt<=6144.0f)
	{
		efkol=(6144.0f - (float)prt)*(440.0f/2048.0f);
		apath1=sin(((float)prt-320.0f)/256.0f)*((6144.0f-(float)prt)/12.0f);
	}

	if (prt>=8192.0f && prt<22528.0f)
	{
		float bsize=32.0f, bzoom=-32.0f;
		float asize=bsize*(((float)prt-8192.0f)/1024.0f);
		if (prt>=9216.0f) asize=bsize;
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_COLOR);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glColor4ub(255-64,255-64,255-128,0);
			glVertex3f(-asize, -bsize/3.0f, bzoom);
			glVertex3f(asize, -bsize/3.0f, bzoom);
			glVertex3f(asize, bsize/3.0f, bzoom);
			glVertex3f(-asize, bsize/3.0f, bzoom);
		glEnd();
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glColor4ub(255,255,255,0);
	}

	if (prt>=6144.0f && prt<8192.0f)
	{
		for (int i=0; i<7; i++)
		{
			fx0[i]=sin(i)*64.0f;
			fy0[i]=cos(i)*64.0f;
			fx1[i]=0.0f;
			fy1[i]=0.0f;
			fdx[i]=(float)(fx1[i]-fx0[i])/1024.0f;
			fdy[i]=(float)(fy1[i]-fy0[i])/1024.0f;
		}
	}

	if (prt>=8192.0f && prt<9216.0f)
	{
		for (int i=0; i<7; i++)
		{
			fontx[i]=fx0[i] + fdx[i]*(prt-8192.0f);
			fonty[i]=fy0[i] + fdy[i]*(prt-8192.0f);
		}
	}

	if (prt>=8192.0f && prt<22528.0f)
	{
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		char *demoname="CloseGL";

		glColor4ub(255,255,255,255);
		for (int i=0; i<7; i++)
			VS_FontWrite(*demoname++, sin((prt+i*128.0f)/256.0f)*8.0f+i*4.0f-8.0f + fontx[i], cos((prt+i*32.0f)/256.0f)*4.0f+4.0f + fonty[i], -30.0f);
		glDisable(GL_TEXTURE_2D);
	}

	if (prt>16384.0f && prt<=19000.0f)
	{
		yspd+=yacl;
		efkol+=yspd;
		apath1=efkol/6.0f;
	}

	if (prt>18800.0f && prt<22528.0f)
	{
		float bsize=32.0f, bzoom=-32.0f;
		float asize=bsize;
		float d3=(22528.0-(float)prt)/(22528.0f-18880.0f);
		if (d3<0.0f) d3=0.0f;

		glColor4f(0.5f, 0.0f, 0.25f, 0.0f);

		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glVertex3f(-asize, -bsize/3.0f*d3, bzoom);
			glVertex3f(asize, -bsize/3.0f*d3, bzoom);
			glVertex3f(asize, -512.0f, bzoom);
			glVertex3f(-asize, -512.0f, bzoom);

			glVertex3f(-asize, 512.0f, bzoom);
			glVertex3f(asize, 512.0f, bzoom);
			glVertex3f(asize, bsize/3.0f*d3, bzoom);
			glVertex3f(-asize, bsize/3.0f*d3, bzoom);
		glEnd();
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glColor4ub(255,255,255,0);
	}

	if (prt>22528.0f)
	{
		partime=SDL_GetTicks();
		npart=2;
	}

	if (prt>=4096.0f && prt<20480.0f)
	{
	ssine s0,s1;

	s0.sinediv=8.0f; s0.sinemul=6.0f; s0.sineadd=-24.0f+sin(SDL_GetTicks()/256.0f)*8.0f; s0.speed=64.0f;
	s1.sinediv=6.0f; s1.sinemul=4.0f; s1.sineadd=24.0f+sin(SDL_GetTicks()/192.0f)*12.0f; s1.speed=48.0f;

	glLoadIdentity();
	glTranslatef(-efkol,0.0f,0.0f);
	VS_Tail(512.0f, 32, s0, s1, 0.5f, 0.25f, 0.1f);

	s0.sinediv=5.0f; s0.sinemul=7.0f; s0.sineadd=-96.0f+sin(SDL_GetTicks()/384.0f)*18.0f; s0.speed=54.0f;
	s1.sinediv=7.0f; s1.sinemul=8.0f; s1.sineadd=-48.0f+sin(SDL_GetTicks()/128.0f)*3.0f; s1.speed=48.0f;

	glLoadIdentity();
	glTranslatef(efkol,0.0f,0.0f);
	VS_Tail(512.0f, 32, s0, s1, 0.1f, 0.3f, 0.25f);

	s0.sinediv=6.0f; s0.sinemul=12.0f; s0.sineadd=48.0f+sin(SDL_GetTicks()/212.0f)*4.0f; s0.speed=44.0f;
	s1.sinediv=4.0f; s1.sinemul=10.0f; s1.sineadd=96.0f+sin(SDL_GetTicks()/234.0f)*10.0f; s1.speed=38.0f;

	glLoadIdentity();
	glTranslatef(efkol,0.0f,0.0f);
	VS_Tail(512.0f, 32, s0, s1, 0.0f, 0.65f, 1.0f);
	}

	glLoadIdentity();
	glTranslatef(0.0f,apath1,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	if (prt>4096.0f) VS_FlatGrid();

	glLoadIdentity();
	glTranslatef(0.0f,0.0f,tz);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glColor3ub(255,255,255);

	if (prt<4096.0f)
		VS_Blob(0.0f, 0.0f, -1, 255, 255, 255, (1024.0f-prt/4.0f));  //8-1024
}


void P_Polar()
{

	int prt=SDL_GetTicks()-partime;

	if (prt>16384.0f && prt<20480.0f)
	{
		amg=64.0f+((float)prt-16384.0f)*(256.0f/4096.0f);
	}

	if (prt>=20480.0f)
	{
		partime=SDL_GetTicks();
		npart=3;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );

	glLoadIdentity();
	glTranslatef(0.0f,0.0f,0.0f);
	glRotatef(sin(SDL_GetTicks()/2048.0f)*192.0f, 0.0f,0.0f,1.0f);

	VS_Floor(256.0f, 6144.0f, 4096.0f, 32.0f);
	VS_Floor(-256.0f, 6144.0f, 4096.0f, 32.0f);

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	VS_Stars2d(0.0f, 8.0f, 0, 256);
	VS_Stars2d(0.0f, 16.0f, 256, 512);
	VS_Stars2d(0.0f, 32.0f, 512, 768);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,tz);

	GC_Cube(32);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	for (int i=0; i<6; i++)
	{
		if (side[i])
		{
			GC_Polar(i);
			VS_FlatGridNew(i);
		}
	}

	glLoadIdentity();
	glTranslatef(0.0f,0.0f,0.0f);
	glRotatef(sin(SDL_GetTicks()/2048.0f)*192.0f, 0.0f,0.0f,1.0f);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	VS_Stars2d(0.0f, amg, 768, 1024);
	for (i=0; i<1024; i++)
	{
		star[i].z+=star[i].speed;
		if (star[i].z>=0) star[i].z=-256.0f;
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);


	glLoadIdentity();

	if (prt>512.0f && prt<2048.0f)
	{
		float rbz=(prt-512.0f)/10.0f;
		glRotatef(rbz,0.0f,0.0f,1.0f);
	}

	if (prt<2048.0f)
	{
		float antere=(float)prt/32.0f;
		float barsize=32.0f, barzoom=-16.0f;
		float diaf=(2048.0f-(float)prt)/2048.0f;

		glDisable(GL_TEXTURE_2D);
		glColor4f(0.5f, 0.0f, 0.25f, diaf);

		for (i=-2; i<3; i+=2)
		{
			glBegin(GL_QUADS);
				glVertex3f(-barsize-antere, -barsize/24.0f + i * (barsize/12.0f), barzoom);
				glVertex3f(barsize-antere, -barsize/24.0f + i * (barsize/12.0f), barzoom);
				glVertex3f(barsize-antere, barsize/24.0f + i * (barsize/12.0f), barzoom);
				glVertex3f(-barsize-antere, barsize/24.0f + i * (barsize/12.0f), barzoom);
			glEnd();
		}

		glColor4f(0.5f, 0.0f, 0.25f, diaf);
		for (i=-3; i<4; i+=2)
		{
			glBegin(GL_QUADS);
				glVertex3f(-barsize+antere, -barsize/24.0f + i * (barsize/12.0f), barzoom);
				glVertex3f(barsize+antere, -barsize/24.0f + i * (barsize/12.0f), barzoom);
				glVertex3f(barsize+antere, barsize/24.0f + i * (barsize/12.0f), barzoom);
				glVertex3f(-barsize+antere, barsize/24.0f + i * (barsize/12.0f), barzoom);
			glEnd();
		}

		glEnable(GL_TEXTURE_2D);
	}
}


void P_Dots()
{

	float ro, phi, theta;

	ro=128.0f;
	phi=SDL_GetTicks()/1024.0f;
	theta=SDL_GetTicks()/1024.0f;

	LightPosition[0] = ro * sin(phi) * cos(theta);
	LightPosition[1] = ro * sin(phi) * sin(theta);
	LightPosition[2] = ro * cos(phi);

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);	
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);

	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	VS_ObjectShow(2);

}

void P_Stars()
{
	glClearColor(0.0f, 0.0f, 0.25f, 0.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glDisable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );

	glEnable(GL_POINT_SPRITE_ARB);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[6]);

	glLoadIdentity();

	glTranslatef(0.0f,0.0f,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);


	for (int i=0; i<1024; i++)
	{
		star[i].z+=star[i].speed*mmo/2;
		if (star[i].z>=0) star[i].z=-256.0f;
	}

	VS_Stars3d();
	for (i=0; i<NSHAPES; i++)
	{
		shape[i].zfp=-((int)((i*(256/NSHAPES))+SDL_GetTicks()/32.0f)%256);
		VS_Flower(shape[i]);
	}

	glDisable(GL_POINT_SPRITE_ARB);

}


void P_Water()
{

	int prt=SDL_GetTicks()-partime;

	if ((SDL_GetTicks()-wft)>32)
	{	wft=SDL_GetTicks();
		kl++;
		GC_Water();
	}


	float pi=3.14151693;
	float d2r=180.0/pi;

	LightAmbient[0] = 0.5f;
	LightAmbient[1] = 0.5f;
	LightAmbient[2] = 1.0f;

	LightPosition[0] = 0.0f;
	LightPosition[1] = 0.0f;
	LightPosition[2] = 128.0f;

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);	
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);

	glDisable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);

	glDisable(GL_BLEND);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );

	glEnable(GL_LIGHT1);
	glLoadIdentity();

	glTranslatef(0.0f,0.0f,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	VS_Pisina(8.0f, -32.0f);

	// Duck

	LightDiffuse[0] = 1.0f;
	LightDiffuse[1] = 1.0f;
	LightDiffuse[2] = 0.0f;

	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);

	glEnable(GL_LIGHTING);

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glTranslatef(ducktx, duckty, ducktz);

	glRotatef(duckx,1.0f,0.0f,0.0f);		//ypsos
	glRotatef(ducky,0.0f,1.0f,0.0f);		//tilt
	glRotatef(duckz,0.0f,0.0f,1.0f);		//direction

	glColor3f(1.0f, 1.0f, 0.0f);
	VS_ObjectShow(2);

	glLoadIdentity();

	glTranslatef(0.0f,0.0f,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	LightDiffuse[0] = 1.0f;
	LightDiffuse[1] = 1.0f;
	LightDiffuse[2] = 1.0f;

	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	VS_Water(2,SDL_GetTicks()/8192.0f,SDL_GetTicks()/16384.0f);


	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	if (prt<8192.0f)
	{
		float ablend;
		for (int y=0; y<6; y++)
			for (int x=0; x<6; x++)
			{
				ablend=(8192.0f-(float)prt)/8192.0f;
				blockblend[x][y]=(float)(x+y+8-((float)prt/8192.0f)*32.0f)/4.0f-ablend;
				if (blockblend[x][y]>1.0f) blockblend[x][y]=1.0f;
				if (blockblend[x][y]<0.0f) blockblend[x][y]=0.0f;

				glLoadIdentity();
				glTranslatef(-(x-3)*4.0f, -(y-3)*3.0f, blockblend[x][y]);
				glRotatef(90.0f*(1.0f - blockblend[x][y]),0.0f,0.0f,1.0f);
				glRotatef(90.0f*(1.0f - blockblend[x][y]),0.0f,1.0f,0.0f);

				VS_Tile(-4.0f, -3.0f, blockblend[x][y]);
			}
	}

	if (prt>=8192.0f && prt<16384.0f)
	{
		controls=true;
		rx+=0.05f;
		tz-=0.1f;
	}

	if (prt>=14336.0f && prt<24536.0f)
	{
		rz=sin(((float)prt-14336.0f)/1024.0f)*32.0f;
		rx=rx-sin(((float)prt-14336.0f)/840.0f)*0.3f;
		tz=tz+sin(((float)prt-14336.0f)/2048.0f)*0.2f;
	}

	if (prt>=24536.0f && prt<32768)
	{
		tz+=0.15f;
	}

	if (prt>=32768.0f && prt<36864.0f)
	{
		float fad2=(36864.0f-(float)prt)/4096.0f;
		if (fad2>=1.0f) fad2=0.95f;
		if (fad2<=0.0f) fad2=0.05f;
		glLoadIdentity();

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_COLOR);
		glEnable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		VS_Fade1(fad2*255.0f, 16.0f);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}

	if (prt>=36864.0f)
	{
		controls=false;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		npart=4;
		partime=SDL_GetTicks();
	}

	glLoadIdentity();
	glTranslatef(0.0f,0.0f,tz);
	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

}


void P_Spherical()
{
	int prt=SDL_GetTicks()-partime;

	float ty,epi;
	
	if (prt<4096.0f)
	{
		epi=(4096.0f-(float)prt)/4096.0f+0.1f;
		ty=cos((float)prt/256.0f)*192.0f*epi;
	}

	if (prt>=4096.0f)
	{
		epi=0.1f;
		ty=cos((float)prt/256.0f)*192.0f*epi;
	}

	GC_Spherical();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );

	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glLoadIdentity();

	glTranslatef(0.0f,ty,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	VS_Spherical();

	if (prt>8192.0f)
	{
	float div1=2.0f, div2=10.0f;
	float mul1=-4.0f;
	float ficken=-8.0f;
	float agh=-42.0f;
	float adist=64.0f;

		glEnable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glLoadIdentity();
		float yt=((float)prt-13000.0f)/64.0f;
		glColor4ub(255,255,255,255);
		VS_TextWrite(" Nasty Bugs presented "+0, agh, yt/div1, sin(yt/div2)*mul1 - adist);
		VS_TextWrite("an OpenGL demonstration"+0, agh, yt/div1+ficken, sin((yt+4.0f)/div2)*mul1 - adist);
		VS_TextWrite("       entitled       "+0, agh, yt/div1+2*ficken, sin((yt+8.0f)/div2)*mul1 - adist);

		VS_TextWrite("       CloseGL       "+0, agh, yt/div1+5*ficken, sin((yt+8.0f)/div2)*mul1 - adist);


		VS_TextWrite("        Code         "+0, agh, yt/div1+9*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("       Optimus       "+0, agh, yt/div1+10*ficken, sin((yt+8.0f)/div2)*mul1 - adist);

		VS_TextWrite("        Music        "+0, agh, yt/div1+12*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("      Apomakros      "+0, agh, yt/div1+13*ficken, sin((yt+8.0f)/div2)*mul1 - adist);


		VS_TextWrite("   Greets goes to    "+0, agh, yt/div1+18*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("       The Lab       "+0, agh, yt/div1+20*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("         ASD         "+0, agh, yt/div1+21*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("        Vortex       "+0, agh, yt/div1+22*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("        Nlogn        "+0, agh, yt/div1+23*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("      Sense Amok     "+0, agh, yt/div1+24*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("  Reversed Engineers "+0, agh, yt/div1+25*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("        Deus         "+0, agh, yt/div1+26*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("       Psyxes        "+0, agh, yt/div1+27*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("      Valladay       "+0, agh, yt/div1+28*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("     Demaniacs       "+0, agh, yt/div1+29*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("        Jobo         "+0, agh, yt/div1+30*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("       Radiant       "+0, agh, yt/div1+31*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("      xperience      "+0, agh, yt/div1+32*ficken, sin((yt+8.0f)/div2)*mul1 - adist);
		VS_TextWrite("        gfx          "+0, agh, yt/div1+33*ficken, sin((yt+8.0f)/div2)*mul1 - adist);



		VS_TextWrite("You may press Esc now"+0, agh, yt/div1+40*ficken, sin((yt+8.0f)/div2)*mul1 - 64.0f);
	}

	if (prt>65536.0f)
	{
		npart=0;
	}
}

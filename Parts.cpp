#ifdef _MSC_VER
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#endif

#include <math.h>
#include <SDL/SDL.h>

#include "opengl.h"

#include "GridCalcs.h"
#include "VertexSend.h"
#include "precalcs.h"
#include "Parts.h"
#include "Script.h"


GLfloat LightAmbient[]= { 0.5f, 0.5f, 1.0f, 1.0f };
GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[]= { 0.0f, 0.0f, 2.0f, 1.0f };

extern bool pointSpritesSupported;

extern bool side[6];
extern GLuint texture[9];
extern bool controls;
extern bool apath2;

extern int globalTime;

extern float rx,ry,rz;
extern float tx,ty,tz;
float tz0,rx0;
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

extern char rcol[nblob],gcol[nblob],bcol[nblob];

extern char fconv[256];
extern int partime;

	float efkol;
	float efkol0;
	float amg=64.0f;
extern int npart;


star3d star[1024];
flower shape[32];

float yspd=0.0f, yacl=0.01f;
float yspd0=0.0f;

void P_Distort()
{
	int prt=globalTime-partime;

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

	float amen=4096.0f-3072.0f;

	if (prt>=12288+amen && prt<20480+amen)
	{
		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_COLOR);
		glEnable(GL_BLEND);
		glLoadIdentity();
		VS_Fade1(((20480.0f+amen-(float)prt)/(20480.0f-12288.0f))*255,-64.0f);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
	}

	if (prt>=20480+amen)
		quit_demo(0);

}


void P_Plasma()
{

	int prt=globalTime-partime;

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
		if (prt>=10480.0f) asize=bsize;
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
//			fx0[i]=sin(i)*64.0f;
//			fy0[i]=cos(i)*64.0f;
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
		const char *demoname="CloseGL";

		glBegin(GL_QUADS);

		glColor4ub(255,255,255,255);
		for (int i=0; i<7; i++) {
			VS_FontWrite(*demoname++, sin((prt+i*128.0f)/256.0f)*8.0f+i*4.0f-8.0f + fontx[i], cos((prt+i*32.0f)/256.0f)*4.0f+4.0f + fonty[i], -30.0f);
		}

		glEnd();

		glDisable(GL_TEXTURE_2D);
	}

	if (prt<=16384.0f)
	{
		//efkol0=0.0f;
		//yspd0=0.0f;
	}

	if (prt>16384.0f && prt<=20480.0f)
	{
		yspd=yspd0 + yacl*(((float)prt-16384.0f)/4.0f);
		efkol=efkol0 + yspd*(((float)prt-16384.0f)/16.0f);
		apath1=efkol/6.0f;
	}


	float flicks=1024.0f;

	if (prt>18560.0f && prt<22528.0f+flicks)
	{
		float bsize=32.0f, bzoom=-32.0f;
		float asize=bsize;
		float d3=(22528.0-(float)prt)/(22528.0f-18560.0f);
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

	if (prt>=22528.0f+flicks)
	{
		partime=SDL_GetTicks();
		npart=2;
	}

	if (prt>=4096.0f && prt<20480.0f)
	{
	ssine s0,s1;

	s0.sinediv=8.0f; s0.sinemul=6.0f; s0.sineadd=-24.0f+sin(globalTime/256.0f)*8.0f; s0.speed=64.0f;
	s1.sinediv=6.0f; s1.sinemul=4.0f; s1.sineadd=24.0f+sin(globalTime/192.0f)*12.0f; s1.speed=48.0f;

	glLoadIdentity();
	glTranslatef(-efkol,0.0f,0.0f);
	VS_Tail(512.0f, 32, s0, s1, 0.5f, 0.25f, 0.1f);

	s0.sinediv=5.0f; s0.sinemul=7.0f; s0.sineadd=-96.0f+sin(globalTime/384.0f)*18.0f; s0.speed=54.0f;
	s1.sinediv=7.0f; s1.sinemul=8.0f; s1.sineadd=-48.0f+sin(globalTime/128.0f)*3.0f; s1.speed=48.0f;

	glLoadIdentity();
	glTranslatef(efkol,0.0f,0.0f);
	VS_Tail(512.0f, 32, s0, s1, 0.1f, 0.3f, 0.25f);

	s0.sinediv=6.0f; s0.sinemul=12.0f; s0.sineadd=48.0f+sin(globalTime/212.0f)*4.0f; s0.speed=44.0f;
	s1.sinediv=4.0f; s1.sinemul=10.0f; s1.sineadd=96.0f+sin(globalTime/234.0f)*10.0f; s1.speed=38.0f;

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
	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glColor3ub(255,255,255);

	if (prt<4096.0f)
	{
		VS_Blob_Begin();
		VS_Blob(0.0f, 0.0f, -1.0f, 255, 255, 255, (1024.0f-prt/4.0f));  //8-1024
		VS_Blob_End();
	}
}


void P_Polar()
{
	static int prevT = globalTime;
	static bool updateStars;
	int i;

	updateStars = false;
	if ((globalTime-prevT)>20)
	{	prevT=SDL_GetTicks();
		updateStars = true;
	}

	float frix1=20480.0f - 2048.0f+1024.0f-256.0f-128.0f-128.0f;
	float frix2=24576.0f - 2048.0f+1024.0f-256.0f-128.0f-128.0f;

	int prt=globalTime-partime;

	if (prt>frix1 && prt<frix2)
	{
		amg=64.0f+((float)prt-frix1)*(256.0f/4096.0f);
	}

	if (prt>=frix2)
	{
		partime=SDL_GetTicks();
		npart=3;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );

	glLoadIdentity();
	glTranslatef(0.0f,0.0f,0.0f);
	glRotatef(sin(globalTime/2048.0f)*192.0f, 0.0f,0.0f,1.0f);

	VS_Floor(256.0f, 6144.0f, 4096.0f, 32.0f);
	VS_Floor(-256.0f, 6144.0f, 4096.0f, 32.0f);

	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_COLOR);	// That would have looked better when going to white, but I keep the original visuals as it should, no enhancement here (I better do a new demo for this)
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glDisable(GL_DEPTH_TEST);
	VS_Stars2d(0.0f, 8.0f, 0, 256, updateStars);
	VS_Stars2d(0.0f, 16.0f, 256, 512, updateStars);
	VS_Stars2d(0.0f, 32.0f, 512, 768, updateStars);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();
	glTranslatef(0.0f,0.0f,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	GC_Cube(32);

	glLoadIdentity();
	glTranslatef(0.0f,0.0f,0.0f);
	glRotatef(sin(globalTime/2048.0f)*192.0f, 0.0f,0.0f,1.0f);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	VS_Stars2d(0.0f, amg, 768, 1024, updateStars);
	for (i=0; i<1024; i++)
	{
		star[i].z +=star[i].speed;
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

void P_Stars()
{
	static int prevT = globalTime;
	static bool updateStars;
	int i;

	updateStars = false;
	if ((globalTime-prevT)>20)
	{	prevT=SDL_GetTicks();
		updateStars = true;
	}

	glClearColor(0.0f, 0.0f, 0.25f, 0.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );

	glLoadIdentity();

	glTranslatef(0.0f,0.0f,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	for (i=0; i<1024; i++)
	{
		//star[i].z+=star[i].speed*mmo/4.0f;
		if (updateStars) star[i].z+=8.0f*star[i].speed;
		if (star[i].z>=0) star[i].z=-256.0f;
	}

	VS_Prepare_Blob_TC(1024);	// 1024=STARS, overtakes 360=FLOWER POINTS

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[5]);

	VS_Stars3d();

	if (pointSpritesSupported) {
		glEnable(GL_POINT_SPRITE_ARB);
		glPointSize(2);
	}
	for (i=0; i<NSHAPES; i++)
	{
		shape[i].zfp = -fmodf(shape[i].zfpBase + globalTime/32.0f, 256.0f);
		VS_Flower(shape[i]);
	}
	if (pointSpritesSupported) {
		glDisable(GL_POINT_SPRITE_ARB);
	}
}


void P_Water()
{
	bool updateCameraRot = false;
	int prt=globalTime-partime;

	if ((globalTime-wft)>20)
	{	wft=SDL_GetTicks();
		kl++;
		GC_Water();
		updateCameraRot = true;
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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
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
	glDisable(GL_CULL_FACE);
	VS_Water(2,globalTime/8192.0f,globalTime/16384.0f);


	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	if (prt<8192.0f)
	{
		float ablend;
		for (int y=0; y<6; y++) {
			for (int x=0; x<6; x++) {
				ablend=(8192.0f-(float)prt)/8192.0f;
				blockblend[x][y]=(float)(x+y+8-((float)prt/8192.0f)*32.0f)/4.0f-ablend;
				if (blockblend[x][y]>1.0f) blockblend[x][y]=1.0f;
				if (blockblend[x][y]<0.0f) blockblend[x][y]=0.0f;

				glLoadIdentity();
				glTranslatef(-(x-3)*4.0f, -(y-3)*3.0f, blockblend[x][y]);
				glRotatef(90.0f*(1.0f - blockblend[x][y]),0.0f,0.0f,1.0f);
				glRotatef(90.0f*(1.0f - blockblend[x][y]),0.0f,1.0f,0.0f);

				glBegin(GL_QUADS);
				glColor4f(1.0f, 1.0f, 1.0f, blockblend[x][y]);
				glVertex3f(-4.0f, -3.0f, -15.0f);
				glVertex3f(0.0f, -3.0f, -15.0f);
				glVertex3f(0.0f, 0.0f, -15.0f);
				glVertex3f(-4.0f, 0.0f, -15.0f);
				glEnd();
			}
			tz0=tz;
			rx0=rx;
		}
	}
	glEnable(GL_LIGHTING);

	if (prt>=8192.0f && prt<16384.0f)
	{
		apath2=false;
		if (updateCameraRot) {
			rx=rx0 + 0.00175f*((float)prt-8192.0f);
			tz=tz0 - 0.0075f*((float)prt-8192.0f);
		}
	}

	if (prt>=14336.0f && prt<24536.0f)
	{
		if (updateCameraRot) {
			rz=sin(((float)prt-14336.0f)/1024.0f)*32.0f;
			rx=rx-sin(((float)prt-14336.0f)/940.0f)*0.3f;
			tz=tz+sin(((float)prt-14336.0f)/2048.0f)*0.2f;
		}
		if (prt>=16384.0f) tz0=tz;
	}

	if (prt>=24536.0f && prt<32768.0f)
	{
		if (updateCameraRot) {
			tz=tz0 + 0.015f*((float)prt-24536.0f);
		}
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
		apath2=true;
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
	static bool test = true;
	int prt=globalTime-partime;

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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glEnable(GL_TEXTURE_2D);

	glLoadIdentity();

	glTranslatef(0.0f,ty,tz);

	glRotatef(rx,1.0f,0.0f,0.0f);
	glRotatef(ry,0.0f,1.0f,0.0f);
	glRotatef(rz,0.0f,0.0f,1.0f);

	VS_Spherical();

	if (prt>8192.0f)
	{
	float div1=6.0f, div2=12.0f;
	float mul1=-4.0f;
	float ficken=-2.5f;
	float agh=-139.0f;
	float adist=24.0f;

	float vlak1=1.0f;

		glEnable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glDisable(GL_CULL_FACE);

		glLoadIdentity();
		glTranslatef(128.0f,0.0f,0.0f);
		float yt=((float)prt-15000.0f)/64.0f;
		glColor4ub(255,255,255,255);

		VS_TextWrite2(" Nasty Bugs presented"+0, agh, yt/div1, sin(yt/div2)*mul1 - adist);
		VS_TextWrite2("an OpenGL demonstration"+0, agh, yt/div1+ficken, sin((yt+4.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("        entitled     "+0, agh, yt/div1+2*ficken, sin((yt+8.0f*vlak1)/div2)*mul1 - adist);

		VS_TextWrite2("       CloseGL       "+0, agh, yt/div1+5*ficken, sin((yt+20.0f*vlak1)/div2)*mul1 - adist);


		VS_TextWrite2("        Code         "+0, agh, yt/div1+9*ficken, sin((yt+36.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       Optimus       "+0, agh, yt/div1+10*ficken, sin((yt+40.0f*vlak1)/div2)*mul1 - adist);

		VS_TextWrite2("        Music        "+0, agh, yt/div1+12*ficken, sin((yt+48.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("      Apomakros      "+0, agh, yt/div1+13*ficken, sin((yt+52.0f*vlak1)/div2)*mul1 - adist);


		VS_TextWrite2("   Greets goes to    "+0, agh, yt/div1+18*ficken, sin((yt+72.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       The Lab       "+0, agh, yt/div1+20*ficken, sin((yt+80.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("         ASD         "+0, agh, yt/div1+21*ficken, sin((yt+84.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("        Vortex       "+0, agh, yt/div1+22*ficken, sin((yt+88.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("        Nlogn        "+0, agh, yt/div1+23*ficken, sin((yt+92.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("      Sense Amok     "+0, agh, yt/div1+24*ficken, sin((yt+96.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("  Reversed Engineers "+0, agh, yt/div1+25*ficken, sin((yt+100.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("        Deus         "+0, agh, yt/div1+26*ficken, sin((yt+104.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       Psyxes        "+0, agh, yt/div1+27*ficken, sin((yt+108.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("      Valladay       "+0, agh, yt/div1+28*ficken, sin((yt+112.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("     Demaniacs       "+0, agh, yt/div1+29*ficken, sin((yt+116.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("        Jobo         "+0, agh, yt/div1+30*ficken, sin((yt+120.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       Radiant       "+0, agh, yt/div1+31*ficken, sin((yt+124.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("      xperience      "+0, agh, yt/div1+32*ficken, sin((yt+128.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("        gfx          "+0, agh, yt/div1+33*ficken, sin((yt+132.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("     Dirty Minds     "+0, agh, yt/div1+34*ficken, sin((yt+136.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       Anubis        "+0, agh, yt/div1+35*ficken, sin((yt+140.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       Texel         "+0, agh, yt/div1+36*ficken, sin((yt+148.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("      SotSoft        "+0, agh, yt/div1+37*ficken, sin((yt+152.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       Stanz         "+0, agh, yt/div1+38*ficken, sin((yt+156.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("      Midiclub       "+0, agh, yt/div1+39*ficken, sin((yt+160.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       AkumaX        "+0, agh, yt/div1+40*ficken, sin((yt+164.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("       Expert        "+0, agh, yt/div1+41*ficken, sin((yt+168.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("     A.Damtsios      "+0, agh, yt/div1+42*ficken, sin((yt+172.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("      Tsoustis       "+0, agh, yt/div1+43*ficken, sin((yt+176.0f*vlak1)/div2)*mul1 - adist);
		VS_TextWrite2("      Relsoft        "+0, agh, yt/div1+44*ficken, sin((yt+180.0f*vlak1)/div2)*mul1 - adist);
	}

	if (prt>65536.0f)
	{
		npart=0;
		partime=SDL_GetTicks();
	}

}

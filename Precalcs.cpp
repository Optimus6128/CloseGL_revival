#ifdef _MSC_VER
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "GridCalcs.h"
#include "precalcs.h"
#include "Parts.h"
#include "VertexSend.h"

//const int gqx=GridX;
//const int gqy=GridY;

int cx=gqx>>1;
int cy=gqy>>1;

// Object Load

float *xo,*yo,*zo;
unsigned short *pp0,*pp1,*pp2;
Vector *pnv;

int ndts,nlns,npls;


// plasma

int fsin1[SIN_SIZE],fsin2[SIN_SIZE],fsin3[SIN_SIZE];
int rgb[256];

// polar

const float rang=512;

const float pi=3.14151693;
const float d2r=180.0/pi;
const float d2b = (rang * d2r) / 360.0;

unsigned char dist[gqx*gqy],angle[gqx*gqy];
int fsin4[SIN_SIZE],fsin5[SIN_SIZE];

extern float starx[1024], stary[1024], starz[1024], starspeed[1024];
extern char starcolr[1024], starcolg[1024], starcolb[1024];

char rcol[nblob],gcol[nblob],bcol[nblob];

//flower

extern star3d star[1024];
extern flower shape[NSHAPES];

//fonts

char fconv[256];

void Precalculations(void)
{

// ===== Plasma Precalcs =====

float l=1;
int i;

for (i=0;i<SIN_SIZE;i++)
	{
		fsin1[i]=sin(i/(l*15.0))*96.0+96.0;
		fsin2[i]=sin(i/(l*20.0))*112.0+112.0;
		fsin3[i]=sin(i/(l*35.0))*128.0+128.0;
	}

for (i=0;i<64;i++)
	{
        rgb[i]=(i<<2)<<16 | (i<<1)<<8;
	    rgb[64+i]=(255-(i<<1))<<16 | ((i<<1)+128)<<8 | i<<2;
	    rgb[128+i]=((63-i)<<1)<<16 | (255-(i<<1))<<8 | 255;
	    rgb[192+i]=(128-(i<<1))<<8 | (63-i)<<2;
	}


// ===== Polar Precalcs =====

	float al=0.25;
	for (i=0;i<SIN_SIZE;i++)
	{
		fsin4[i]=sin(i/(al*d2b))*48.0+64.0;
		fsin5[i]=sin(i/(al*d2b/2))*40.0+48.0;
	}

	i=0;
	for (int y=-cy;y<cy;y++)
	   for (int x=-cx;x<cx;x++)
	   {
	       dist[i]=(sqrtf(x*x + y*y)*8.0f);
		   angle[i++]=(atan2f(y,x)*d2b);
	   }


// ==== Star Precalcs ====

	for (i=0; i<1024; i++)
	{
		starx[i] = (float)(rand()%2048)-1024.0f;
		stary[i] = (float)(rand()%1024) - 512.0f;
		starz[i] = -384.0f;
		starspeed[i] = ((float)(rand()%16384))/-4096.0f;
		if (starspeed[i]>-0.5f) starspeed[i]=-0.5f;
		starcolr[i] = rand()%192 + 64;
		starcolg[i] = rand()%192 + 64;
		starcolb[i] = rand()%192 + 64;
	}

// ==== Star3d Precalcs ====
	for (i=0; i<1024; i++)
	{
		star[i].x=rand()%256 - 128;
		star[i].y=rand()%256 - 128;
		star[i].z=-rand()%256;
		star[i].rcol=rand()%128 + 128;
		star[i].gcol=rand()%128 + 128;
		star[i].bcol=rand()%128 + 128;
		star[i].size=rand()%224 + 32;
		star[i].speed=(float)(rand()%256)/4096.0f;
	}

// ==== Flower Precalcs ====


	for (i=0; i<NSHAPES; i++)
	{
		shape[i].sinmul1=rand()%4 + 6.0f;
		shape[i].tmul1=rand()%4+1;
		shape[i].angdiv1=rand()%16 + 8.0f;

		shape[i].sinmul2=rand()%5 + 2.0f;
		shape[i].tmul2=rand()%4+1;
		shape[i].angdiv2=rand()%16 + 8.0f;

		shape[i].fsize=rand()%4 + 12.0f;

		shape[i].rdiv=rand()%4 + 1;
		shape[i].gdiv=rand()%4 + 1;
		shape[i].bdiv=rand()%4 + 1;

		shape[i].xfp=rand()%128 - 64;
		shape[i].yfp=rand()%128 - 64;
		shape[i].zfp=-(i*16);

		shape[i].zfpBase = (float)i * (256.0f/NSHAPES);
		shape[i].zfp = -shape[i].zfpBase;
	}

// ==== Font Precalcs ====

	for (i=0; i<256; i++)
		fconv[i]=0;

	for (i=65; i<91; i++)
		fconv[i]=i-64;

	for (i=97; i<123; i++)
		fconv[i]=i-70;

	for (i=49; i<57; i++)
		fconv[i]=i+68;

	fconv['0']=126;
	fconv['!']=63;
}





void BlobColors()
{
	for (int i=0; i<nblob; i++)
	{
		rcol[i]=rand() & 255;
		gcol[i]=rand() & 255;
		bcol[i]=rand() & 255;
	}
}

static unsigned short read_uint16(FILE *fp)
{
	unsigned short val;
	fread(&val, 2, 1, fp);
	if(*(unsigned int*)"ABCD" == 0x41424344) {
		return (val >> 8) | (val << 8);
	}
	return val;
}

static unsigned int read_uint32(FILE *fp)
{
	unsigned int val;
	fread(&val, 4, 1, fp);
	if(*(unsigned int*)"ABCD" == 0x41424344) {
		return (val << 24) | ((val & 0xff00) << 8) | ((val & 0xff0000) >> 8) | (val >> 24);
	}
	return val;
}

void LoadObject2()
{

	FILE *obj3d;
	//float *kurasi;

	if(!(obj3d=fopen("duck.3do","rb"))) {
		fprintf(stderr, "failed to open duck.3do\n");
		abort();
	}

	unsigned char ca,cb,cc,cd;
	unsigned int skata;

	ndts = read_uint16(obj3d);
	npls = read_uint16(obj3d);

	xo = new float[ndts];
	yo = new float[ndts];
	zo = new float[ndts];

	pp0 = new unsigned short[npls];
	pp1 = new unsigned short[npls];
	pp2 = new unsigned short[npls];

	pnv = new Vector[ndts];

	Vector *nv = new Vector[npls];

	int i;
	for (i=0; i<ndts; i++)
	{
		skata = read_uint32(obj3d);
		xo[i]=((float)skata/262144.0f-4096.0f)/128.0f;

		skata = read_uint32(obj3d);
		yo[i]=((float)skata/262144.0f-4096.0f)/128.0f;

		skata = read_uint32(obj3d);
		zo[i]=((float)skata/262144.0f-4096.0f)/128.0f;
	}

	for (i=0; i<npls; i++)
	{
		pp0[i] = read_uint16(obj3d);
		pp1[i] = read_uint16(obj3d);
		pp2[i] = read_uint16(obj3d);
	}

	fclose(obj3d);


	Vector v1,v2;

	for (i=0; i<npls; i++)
	{
		v1.x=xo[pp2[i]]-xo[pp1[i]]; v1.y=yo[pp2[i]]-yo[pp1[i]]; v1.z=zo[pp2[i]]-zo[pp1[i]];
		v2.x=xo[pp1[i]]-xo[pp0[i]]; v2.y=yo[pp1[i]]-yo[pp0[i]]; v2.z=zo[pp1[i]]-zo[pp0[i]];
		nv[i]=Meon(Normalize(CrossProduct(v1,v2)));
	}


	int nvpoly;
	Vector nvsum;

	for (i=0; i<ndts; i++)
	{
		nvpoly=0;
		nvsum.x=0; nvsum.y=0; nvsum.z=0;

		for (int j=0; j<npls; j++)
		{
			if (pp0[j]==i || pp1[j]==i || pp2[j]==i)
			{
				nvpoly++;
				nvsum.x+=nv[j].x; nvsum.y+=nv[j].y; nvsum.z+=nv[j].z;
			}
		}
		pnv[i].x=nvsum.x/nvpoly; pnv[i].y=nvsum.y/nvpoly; pnv[i].z=nvsum.z/nvpoly;
	}

	delete [] nv;
}


Vector CrossProduct(Vector v1, Vector v2)
{
	Vector v;
	v.x=v1.y*v2.z-v1.z*v2.y;
	v.y=v1.z*v2.x-v1.x*v2.z;
	v.z=v1.x*v2.y-v1.y*v2.x;
	//v=Meon(v);
	return v;
}


float DotProduct(Vector v1, Vector v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z+v2.z;
}


Vector Normalize(Vector v)
{
	float d=sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
	v.x=v.x/d;
	v.y=v.y/d;
	v.z=v.z/d;
	return v;
}

Vector Meon(Vector v)
{
	v.x=-v.x;
	v.y=-v.y;
	v.z=-v.z;
	return v;
}

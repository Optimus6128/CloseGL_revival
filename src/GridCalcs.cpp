#include <SDL/SDL.h>
#include <math.h>
#include <stdlib.h>
#include "GridCalcs.h"
#include "precalcs.h"
#include "VertexSend.h"


const int nqx=128;
const int nqy=96; //nqx/ratio; (An einai dynaton!)

extern int partime;

extern unsigned char dist[gqx*gqy];
extern unsigned char angle[gqx*gqy];

extern int fsin1[SIN_SIZE],fsin2[SIN_SIZE],fsin3[SIN_SIZE];
extern int rgb[256];
extern int fsin4[SIN_SIZE],fsin5[SIN_SIZE];

extern int globalTime;


// === Distort variables ===

float gridx[nqx*nqy];
float gridy[nqx*nqy];
float gridz[nqx*nqy];


// === Plasma variables ===

// Commenting out, can use plasma common
/*char gridr[nqx*nqy];
char gridg[nqx*nqy];
char gridb[nqx*nqy];*/

unsigned char plgridr[pqx*pqy];
unsigned char plgridg[pqx*pqy];
unsigned char plgridb[pqx*pqy];
float plgrida[pqx*pqy];


// === Polar variables ===

Color3 pgridc[gqx*gqy];
float pgrida[gqx*gqy];

float gx[gqx*gqy];
float gy[gqx*gqy];
float gz[gqx*gqy];

extern float rx,ry,rz;

// ==== Water variables ====

int buffer1[wqx*wqy];
int buffer2[wqx*wqy];
float hbuffer[wqx*wqy];

float nbufferx[wqx*wqy];
float nbuffery[wqx*wqy];
float nbufferz[wqx*wqy];

int *ba=buffer1+wqx+1,*bb=buffer2+wqx+1;
float *hb=hbuffer+wqx+1;

bool ripple=false;
int ripplex, rippley;
float duckrpx, duckrpy;

extern int kl;

extern int k;
extern int rippletime;
extern bool splash1;
bool splashover=false;

extern float duckx,ducky,duckz,ducktx,duckty,ducktz;
extern bool eba;

float rng=0;

// ==== Spherical variables ====

float sphx[spx*spy];
float sphy[spx*spy];
float sphz[spx*spy];

unsigned char sphr[spx*spy];
unsigned char sphg[spx*spy];
unsigned char sphb[spx*spy];

const float pi=3.14151693f;
const float d2r=180.0f/pi;


// ======== Polar ========

void GC_Polar(int nfx)
{

	unsigned char *pdist=dist;
	unsigned char *pangl=angle;

	int c;
	int i=0;
	for (int y=0; y<gqy; y++)
	{
		for (int x=0; x<gqx; x++)
		{

			switch(nfx)
			{
			case 0:
				c=rgb[(((*pdist)+fsin4[(*pangl)+fsin5[(*pangl)+k]]-(k<<2))&255)];
			break;

			case 1:
				c=rgb[((*pangl)+fsin4[((*pdist)>>1)+(k<<1)])&255];
			break;

			case 2:
				c=rgb[((*pdist)+fsin4[2*(*pangl)+k]+(k<<2))&255];
			break;

			case 3:
				c=rgb[((*pdist)+(*pangl)+(k<<2))&255];
			break;

			case 4:
				c=rgb[((*pdist)+fsin4[(*pdist)+fsin5[(*pangl)+k]]-(k<<2))&255];
			break;

			case 5:
				c=rgb[((*pangl)+fsin4[(*pdist)+fsin5[(*pangl)+k]]-(k<<2))&255];
			break;

			default:
				break;
			}

			pgridc[i].r = (unsigned char)((c & 0x00FF0000)>>16);
			pgridc[i].g =(unsigned char)((c & 0x0000FF00)>>8);
			pgridc[i].b =(unsigned char)c;

			++pdist;
			++pangl;
			++i;
		}
	}
}


// ======== Plasma ========

void GC_Plasma()
{
	int i=0;
	int cp;
	for (int y=0; y<pqy; y++)
	{
		for (int x=0; x<pqx; x++)
		{
			cp=(int)((fsin1[x]+fsin2[y+k]+fsin3[x+y+3*k])*1.5f);
			plgridr[i]=(char)(((unsigned int)rgb[(unsigned char)(cp&255)] & 0x00FF0000)>>16);
			plgridg[i]=(char)(((unsigned int)rgb[(unsigned char)(cp&255)] & 0x0000FF00)>>8);
			plgridb[i]=(char)((unsigned int)rgb[(unsigned char)(cp&255)]);

			plgrida[i++]=(float)(((float)((unsigned char)cp&255))/32.0f);
		}
	}
}




// ======== Distort ========

void GC_Distort()
{
	int x,y;
	int i=0;
	for (y=0; y<nqy; y++) {
		for (x=0; x<nqx; x++)
		{
			int c=rgb[(int)((fsin1[x]+fsin2[y+k]+fsin3[x+y+3*k])*1.5)&255];
			plgridr[i]=(c & 0x00FF0000)>>16;
			plgridg[i]=(c & 0x0000FF00)>>8;
			plgridb[i++]=c;
		}
	}

	i=0;
	for (y=0; y<nqy; y++) {
		const float sinyk = sinf((y+k)/15.0f)*((float)nqx/8.0f);
		const float nqy8 = (float)nqy/8.0f;
		const float sinykk = sinf((y+k+k)/23.0f)*12.0f+60.0f;
		for (x=0; x<nqx; x++)
		{
			gridx[i]=(float)(x-(nqx>>1)) + sinyk;
			gridy[i]=(float)(y-(nqy>>1)) + sinf((x+k)/11.0f)*nqy8;
			gridz[i++]=(float)(sinf((x+k)/42.0f)*8.0f+sinykk);
		}
	}
}



// ===== FlatGrid =====

void GC_FlatGrid(point2d p0, point2d p1, point2d p2, point2d p3)
{

	float npts=gqx;
	float npts2 = npts-1;

	float x0,y0,z0;
	float dx,dy,dz,dx0,dy0,dz0;
	float ddx,ddy,ddz;
	float yp0,dyp0,zp0,dzp0;

	dx=(p1.x-p0.x)/npts2;
	dy0=(p3.y-p0.y)/npts2;
	dz0=(p3.z-p0.z)/npts2;

	dx0=(p3.x-p0.x)/npts2;
	dyp0=dy=(p1.y-p0.y)/npts2;
	dzp0=dz=(p1.z-p0.z)/npts2;

	ddx=((p2.x-p3.x) - (p1.x-p0.x))/(npts2*npts2);
	ddy=((p2.y-p3.y) - (p1.y-p0.y))/(npts2*npts2);
	ddz=((p2.z-p3.z) - (p1.z-p0.z))/(npts2*npts2);

	x0=p0.x;
	yp0=p0.y;
	zp0=p0.z;

	int i=0;
	for (int y=0; y<gqy; y++)
	{
		x0=p0.x + y*dx0;
		y0=yp0;
		z0=zp0;
		for (int x=0; x<gqx; x++)
		{
			gx[i]=x0; gy[i]=y0; gz[i++]=z0;
			x0+=dx;
			y0+=dy;
			z0+=dz;
		}

		dx+=ddx;
		dy+=ddy;
		dz+=ddz;

		yp0+=dy0;
		zp0+=dz0;
	}
}




void GC_Cube(float hx)
{
	point2d p0,p1,p2,p3;

	p0.x=-hx; p0.y=hx; p0.z=hx;
	p1.x=hx; p1.y=hx; p1.z=hx;

	p2.x=hx; p2.y=-hx; p2.z=hx;
	p3.x=-hx; p3.y=-hx; p3.z=hx;

	if (VS_CubeTest(p0, p1, p2, p3, rx, ry, rz)) {
		GC_FlatGrid(p0,p1,p2,p3);
		GC_Polar(0);
		VS_FlatGridNew();
	}


	p0.x=hx; p0.y=hx; p0.z=hx;
	p1.x=hx; p1.y=hx; p1.z=-hx;

	p2.x=hx; p2.y=-hx; p2.z=-hx;
	p3.x=hx; p3.y=-hx; p3.z=hx;

	if (VS_CubeTest(p0, p1, p2, p3, rx, ry, rz)) {
		GC_FlatGrid(p0,p1,p2,p3);
		GC_Polar(1);
		VS_FlatGridNew();
	}


	p0.x=hx; p0.y=hx; p0.z=-hx;
	p1.x=-hx; p1.y=hx; p1.z=-hx;

	p2.x=-hx; p2.y=-hx; p2.z=-hx;
	p3.x=hx; p3.y=-hx; p3.z=-hx;

	if (VS_CubeTest(p0, p1, p2, p3, rx, ry, rz)) {
		GC_FlatGrid(p0,p1,p2,p3);
		GC_Polar(2);
		VS_FlatGridNew();
	}


	p0.x=-hx; p0.y=hx; p0.z=-hx;
	p1.x=-hx; p1.y=hx; p1.z=hx;

	p2.x=-hx; p2.y=-hx; p2.z=hx;
	p3.x=-hx; p3.y=-hx; p3.z=-hx;

	if (VS_CubeTest(p0, p1, p2, p3, rx, ry, rz)) {
		GC_FlatGrid(p0,p1,p2,p3);
		GC_Polar(3);
		VS_FlatGridNew();
	}


	p0.x=-hx; p0.y=hx; p0.z=hx;
	p1.x=-hx; p1.y=hx; p1.z=-hx;

	p2.x=hx; p2.y=hx; p2.z=-hx;
	p3.x=hx; p3.y=hx; p3.z=hx;

	if (VS_CubeTest(p0, p1, p2, p3, rx, ry, rz)) {
		GC_FlatGrid(p0,p1,p2,p3);
		GC_Polar(4);
		VS_FlatGridNew();
	}


	p0.x=-hx; p0.y=-hx; p0.z=-hx;
	p1.x=-hx; p1.y=-hx; p1.z=hx;

	p2.x=hx; p2.y=-hx; p2.z=hx;
	p3.x=hx; p3.y=-hx; p3.z=-hx;

	if (VS_CubeTest(p0, p1, p2, p3, rx, ry, rz)) {
		GC_FlatGrid(p0,p1,p2,p3);
		GC_Polar(5);
		VS_FlatGridNew();
	}
}

void GC_Water()
{
	int *b1=ba,*b2=bb,*bc;
	float *h=hb;
	unsigned int wh;
	int x,y;

    bc=bb; bb=ba; ba=bc;


	if ((kl & 7)==0) *(b1 + rand()%wqx + (rand()%((int)(wqy*0.8))+(int)(wqy*0.1))*wqx)=4096;


	if (ripple)
	{

	 float riplx = (float)cos((globalTime-rippletime)/700.0f);
	 float riply = (float)sin((globalTime-rippletime)/700.0f);

	 if (rng<1) rng+=0.01f;
	 ducktz=10.0f;
	 float adist=(float)sqrt(riplx*riplx+riply*riply);
	 duckz=(float)atan2(riply,riplx)*d2r*adist;

		if (eba)
		{
			duckrpx=(float)wqx/2.0f + riplx * ((float)wqx/3.0f - 1.0f)*rng;
			duckrpy=(float)wqy/2.0f + riply * ((float)wqy/3.0f - 1.0f)*rng;

 			ripplex=(int)duckrpx;
 			rippley=(int)duckrpy;

			*(b1 + ripplex + rippley*wqx)=512;
		}

		else
		{
			ripplex=(int)((float)wqx/2.0f);
			rippley=(int)((float)wqy/2.0f);
			duckrpx=(float)ripplex;
			duckrpy=(float)rippley;
			duckz=0.0f;
		}
	}
	else
	{
		ripplex=(int)((float)wqx/2.0f);
		rippley=(int)((float)wqy/2.0f);
		duckrpx=(float)ripplex;
		duckrpy=(float)rippley;
		duckz=0.0f;
	}

	if (splashover) splash1=false;
	if (splash1)
	{
		int splashx, splashy;
		splashx=wqx/2;
		splashy=wqy/2;
		
		for (int y=-8; y<8; y++)
			for (int x=-8; x<8; x++)
				*(b1+splashx+x+(splashy+y)*wqx)=(int)(sqrtf((float)(x*x*x*x+y*y*y*y))*16.0f);
		splashover=true;
	}


	for (y=1; y<wqy-1; y++)
	{
		for (x=1; x<wqx-1; x++)
		{
			wh=abs(( ( *(b1-1) + *(b1+1) + *(b1-wqx) + *(b1+wqx) + *(b1-1-wqx) + *(b1-1+wqx) + *(b1+1-wqx) + *(b1+1+wqx))>>2) - *b2);
			//wh=abs((( ( *(b1-1) + *(b1+1) + *(b1-wqx) + *(b1+wqx) + *(b1-1-wqx) + *(b1-1+wqx) + *(b1+1-wqx) + *(b1+1+wqx) + *b1)/4.5f)) - *b2);

		if (wh>384) wh=384;
		if (x==1 || x==(wqx-2) || y==1 || y==(wqy-2)) wh=0;
		
		*b2++=wh;
		b1++;
		*h++= wh/24.0f;

		}
		b1+=2;
		b2+=2;
		h+=2;
	}


	int dx00,dy00;
	int dx10,dy10;

	int i=wqx+1;
	for (y=1; y<wqy-1; y++)
	{
		for (x=1; x<wqx-1; x++)
		{
			dx00=(int)(hbuffer[i-1]-hbuffer[i]); dx10=(int)(hbuffer[i]-hbuffer[i+1]);
			dy00=(int)(hbuffer[i-wqx]-hbuffer[i]); dy10=(int)(hbuffer[i]-hbuffer[i+wqx]);

			nbufferx[i]=(dx00+dx10)/2.0f;
			nbuffery[i]=(dy00+dy10)/2.0f;
			nbufferz[i]=0.5; //sqrt(1 - nbufferx[i]*nbufferx[i] - nbuffery[i]*nbuffery[i]);
			i++;
		}
		i+=2;
	}
}


static float sinPhiTab[180];
static float cosPhiTab[180];

static void initGC_Spherical()
{
	int i=0;
	for (float phi=0.0f; phi<180.0f; phi+=(180.0f/spy))
	{
		sinPhiTab[i] = sinf(phi/d2r);
		cosPhiTab[i] = cosf(phi/d2r);
		++i;
	}
}

void GC_Spherical()
{
	static bool isGC_SphericalInit = false;
	if (!isGC_SphericalInit) {
		initGC_Spherical();
		isGC_SphericalInit = true;
	}

	int i=0;
	float d2r4=d2r*4.0f;
	const float gt = (float)globalTime;
	const float t0 = sinf(gt/276.0f)*0.3f+1.0f;
	const float t1 = sinf(gt/376.0f)*0.2f+1.0f;
	const float t2 = sinf(gt/176.0f)*0.1f+1.0f;
	const unsigned int gtt = globalTime / 32;

	for (float theta=0.0f; theta<360.0f; theta+=(360.0f/spx))
	{
		const float ro0 = sinf((gt+theta*32.0f)/d2r4)*4.0f + 40.0f;
		const float cth = cosf(theta/d2r);
		const float sth = sinf(theta/d2r);
		const float cth0 = cth * t0;
		const float sth1 = sth * t1;
		int n = 0;
		for (float phi=0.0f; phi<180.0f; phi+=(180.0f/spy))
		{
			const float ii = (float)i;
			const float ro = ro0 + sinf((gt+phi*32.0f)/d2r4)*8.0f;

			const float ros = ro * sinPhiTab[n];
			const float px = ros * cth0;
			const float py = ros * sth1;
			const float pz = ro * cosPhiTab[n] * t2;
			++n;

			sphx[i]= px;
			sphy[i]= py;
			sphz[i]= pz;

			sphr[i]=(unsigned char)(px+128+sinf((3*px+sphy[i+(gtt>>1)])/12.0f)*64.0f);
			sphg[i]=(unsigned char)(py+128+sinf((sphx[i+gtt]+2*py)/8.0f)*64.0f);
			sphb[i]=(unsigned char)(px+sinf((px+3*py)/16.0f)*32+sinf((4*px+2*py)/8.0f)*32.0f+128.0f);

			++i;
		}
	}
}

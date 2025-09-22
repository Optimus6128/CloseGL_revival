#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include "GridCalcs.h"
#include "precalcs.h"
#include "VertexSend.h"


const int nqx=128;
const int nqy=96; //nqx/ratio; (An einai dynaton!)

//const int gqx=GridX;
//const int gqy=GridY;

//const int pqx=PlGridX;
//const int pqy=PlGridY;

//const int wqx=W_Width;
//const int wqy=W_Height;


extern unsigned char dist[gqx*gqy];
extern unsigned char angle[gqx*gqy];

extern int fsin1[32768],fsin2[32768],fsin3[32768];
extern int rgb[256];
extern int fsin4[32768],fsin5[32768];


// === Distort variables ===

float gridx[nqx*nqy];
float gridy[nqx*nqy];
float gridz[nqx*nqy];


// === Plasma variables ===

unsigned char gridr[nqx*nqy];
unsigned char gridg[nqx*nqy];
unsigned char gridb[nqx*nqy];

unsigned char plgridr[pqx*pqy];
unsigned char plgridg[pqx*pqy];
unsigned char plgridb[pqx*pqy];
float plgrida[pqx*pqy];


// === Polar variables ===

unsigned char pgridr[gqx*gqy];
unsigned char pgridg[gqx*gqy];
unsigned char pgridb[gqx*gqy];
float pgrida[gqx*gqy];

float gx[16][gqx*gqy];
float gy[16][gqx*gqy];
float gz[16][gqx*gqy];

extern bool side[6];

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

extern kl;

extern i,k,x,y,c;
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

const float pi=3.14151693;
const float d2r=180.0/pi;

// ======== Polar ========

void GC_Polar(int nfx)
{

	unsigned char *pdist=dist;
	unsigned char *pangl=angle;

	i=0;
	for (y=0; y<gqy; y++)
	{
		for (x=0; x<gqx; x++)
		{

			switch(nfx)
			{
			case 0:
				pgrida[i]=(float)(((*pdist++)+fsin4[(*pangl)+fsin5[(*pangl++)+k]]-(k<<2))&255);
				c=rgb[(int)pgrida[i]];
			break;

			case 1:
				pgrida[i]=(float)(((*pangl++)+fsin4[((*pdist++)>>1)+(k<<1)])&255);
				c=rgb[(int)pgrida[i]];
			break;

			case 2:
				pgrida[i]=(float)(((*pdist++)+fsin4[2*(*pangl++)+k]+(k<<2))&255);
				c=rgb[(int)pgrida[i]];
			break;

			case 3:
				pgrida[i]=(float)(((*pdist++)+(*pangl++)+(k<<2))&255);
				c=rgb[(int)pgrida[i]];
			break;

			case 4:
				pgrida[i]=(float)(((*pdist)+fsin4[(*pdist++)+fsin5[(*pangl++)+k]]-(k<<2))&255);
				c=rgb[(int)pgrida[i]];
			break;

			case 5:
				pgrida[i]=(float)(((*pangl)+fsin4[(*pdist++)+fsin5[(*pangl++)+k]]-(k<<2))&255);
				c=rgb[(int)pgrida[i]];
			break;

			default:
				break;
			}

			if (pgrida[i]>127) pgrida[i]=255-pgrida[i];
			pgrida[i]=pgrida[i]/16.0f;
			pgridr[i]=(c & 0x00FF0000)>>16;
			pgridg[i]=(c & 0x0000FF00)>>8;
			pgridb[i++]=c;
		}
	}
}


// ======== Plasma ========

void GC_Plasma()
{
	i=0;
	for (y=0; y<pqy; y++)
		for (x=0; x<pqx; x++)
		{
			c=(int)(fsin1[x]+fsin2[y+k]+fsin3[x+y+3*k])*1.5;
			plgridr[i]=(rgb[c&255] & 0x00FF0000)>>16;
			plgridg[i]=(rgb[c&255] & 0x0000FF00)>>8;
			plgridb[i]=rgb[c&255];

//			if (c>127) c=255-c;
			plgrida[i++]=(c&255)/32.0f;
		}
}




// ======== Distort ========

void GC_Distort()
{
	i=0;
	for (y=0; y<nqy; y++)
		for (x=0; x<nqx; x++)
		{
			c=rgb[(int)((fsin1[x]+fsin2[y+k]+fsin3[x+y+3*k])*1.5)&255];
			gridr[i]=(c & 0x00FF0000)>>16;
			gridg[i]=(c & 0x0000FF00)>>8;
			gridb[i++]=c;
		}

	i=0;
	for (y=0; y<nqy; y++)
		for (x=0; x<nqx; x++)
		{
			gridx[i]=(float)((x-(nqx>>1)) + sin((y+k)/15.0f)*nqx/8.0f);
			gridy[i]=(float)((y-(nqy>>1)) + sin((x+k)/11.0f)*nqy/8.0f);
			gridz[i++]=(float)(sin((x+k)/42.0f)*8.0f+sin((y+k+k)/23.0f)*12.0f+60.0f);
		}
}



// ===== FlatGrid =====

void GC_FlatGrid(point2d p0, point2d p1, point2d p2, point2d p3, int face)
{

	float npts=gqx;

	float x0,y0,z0;
	float dx,dy,dz,dx0,dy0,dz0;
	float ddx,ddy,ddz;
	float yp0,dyp0,zp0,dzp0;

	dx=(p1.x-p0.x)/(npts-2);
	dy0=(p3.y-p0.y)/(npts-2);
	dz0=(p3.z-p0.z)/(npts-2);

	dx0=(p3.x-p0.x)/(npts-2);
	dyp0=dy=(p1.y-p0.y)/(npts-2);
	dzp0=dz=(p1.z-p0.z)/(npts-2);

	ddx=((p2.x-p3.x) - (p1.x-p0.x))/((npts-2)*(npts-2));
	ddy=((p2.y-p3.y) - (p1.y-p0.y))/((npts-2)*(npts-2));
	ddz=((p2.z-p3.z) - (p1.z-p0.z))/((npts-2)*(npts-2));

	x0=p0.x;
	yp0=p0.y;
	zp0=p0.z;

	float boxhgt;

	i=0;
	for (y=0; y<npts; y++)
	{
		x0=p0.x + y*dx0;
		y0=yp0;
		z0=zp0;
		for (x=0; x<npts; x++)
		{
			boxhgt=(pgrida[x+y*gqx]-64)/1.0f;
			gx[face][i]=x0; gy[face][i]=y0; gz[face][i++]=z0;
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

		VS_CubeTest(p0, p1, p2, p3, rx, ry, rz, 0);

		if (side[0])
			GC_FlatGrid(p0,p1,p2,p3,0);


		p0.x=hx; p0.y=hx; p0.z=hx;
		p1.x=hx; p1.y=hx; p1.z=-hx;

		p2.x=hx; p2.y=-hx; p2.z=-hx;
		p3.x=hx; p3.y=-hx; p3.z=hx;

		VS_CubeTest(p0, p1, p2, p3, rx, ry, rz, 1);

		if (side[1])
			GC_FlatGrid(p0,p1,p2,p3,1);


		p0.x=hx; p0.y=hx; p0.z=-hx;
		p1.x=-hx; p1.y=hx; p1.z=-hx;

		p2.x=-hx; p2.y=-hx; p2.z=-hx;
		p3.x=hx; p3.y=-hx; p3.z=-hx;

		VS_CubeTest(p0, p1, p2, p3, rx, ry, rz, 2);

		if (side[2])
			GC_FlatGrid(p0,p1,p2,p3,2);


		p0.x=-hx; p0.y=hx; p0.z=-hx;
		p1.x=-hx; p1.y=hx; p1.z=hx;

		p2.x=-hx; p2.y=-hx; p2.z=hx;
		p3.x=-hx; p3.y=-hx; p3.z=-hx;

		VS_CubeTest(p0, p1, p2, p3, rx, ry, rz, 3);

		if (side[3])
			GC_FlatGrid(p0,p1,p2,p3,3);


		p0.x=-hx; p0.y=hx; p0.z=hx;
		p1.x=-hx; p1.y=hx; p1.z=-hx;

		p2.x=hx; p2.y=hx; p2.z=-hx;
		p3.x=hx; p3.y=hx; p3.z=hx;

		VS_CubeTest(p0, p1, p2, p3, rx, ry, rz, 4);

		if (side[4])
			GC_FlatGrid(p0,p1,p2,p3,4);


		p0.x=-hx; p0.y=-hx; p0.z=-hx;
		p1.x=-hx; p1.y=-hx; p1.z=hx;

		p2.x=hx; p2.y=-hx; p2.z=hx;
		p3.x=hx; p3.y=-hx; p3.z=-hx;

		VS_CubeTest(p0, p1, p2, p3, rx, ry, rz, 5);

		if (side[5])
			GC_FlatGrid(p0,p1,p2,p3,5);

}

void GC_Water()
{
	int *b1=ba,*b2=bb,*bc;
	float *h=hb;
	unsigned int wh;


    bc=bb; bb=ba; ba=bc;


	if ((kl & 7)==0) *(b1 + rand()%wqx + (rand()%((int)(wqy*0.8))+(int)(wqy*0.1))*wqx)=4096;


	if (ripple)
	{
	 float riplx = cos((SDL_GetTicks()-rippletime)/700.0f);
	 float riply = sin((SDL_GetTicks()-rippletime)/700.0f);

	duckrpx=(float)wqx/2.0f + riplx * ((float)wqx/3.0f - 1.0f)*rng;
	duckrpy=(float)wqy/2.0f + riply * ((float)wqy/3.0f - 1.0f)*rng;

 	ripplex=duckrpx;
 	rippley=duckrpy;

    *(b1 + ripplex + rippley*wqx)=512;

		if (eba)
		{
			if (rng<1) rng+=0.01f;
			ducktz=10.0f;
			float adist=sqrt(riplx*riplx+riply*riply);
			duckz=atan2(riply,riplx)*d2r*adist;
		}
	}

	if (splashover) splash1=false;
	if (splash1)
	{
		int splashx, splashy;
		splashx=wqx/2;
		splashy=wqy/2;
		
		for (y=-8; y<8; y++)
			for (x=-8; x<8; x++)
				*(b1+splashx+x+(splashy+y)*wqx)=sqrt(x*x*x*x+y*y*y*y)*16;
		splashover=true;
	}


	for (y=1; y<wqy-1; y++)
	{
		for (x=1; x<wqx-1; x++)
		{
//			wh=abs(( ( *(b1-1) + *(b1+1) + *(b1-wqx) + *(b1+wqx) + *(b1-1-wqx) + *(b1-1+wqx) + *(b1+1-wqx) + *(b1+1+wqx))>>2) - *b2);
			wh=abs(( ( *(b1-1) + *(b1+1) + *(b1-wqx) + *(b1+wqx) + *(b1-1-wqx) + *(b1-1+wqx) + *(b1+1-wqx) + *(b1+1+wqx) + *b1)/4.5) - *b2);

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

	i=wqx+1;
	for (y=1; y<wqy-1; y++)
	{
		for (x=1; x<wqx-1; x++)
		{
			dx00=hbuffer[i-1]-hbuffer[i]; dx10=hbuffer[i]-hbuffer[i+1];
			dy00=hbuffer[i-wqx]-hbuffer[i]; dy10=hbuffer[i]-hbuffer[i+wqx];

			nbufferx[i]=(dx00+dx10)/2.0f;
			nbuffery[i]=(dy00+dy10)/2.0f;
			nbufferz[i]=0.5; //sqrt(1 - nbufferx[i]*nbufferx[i] - nbuffery[i]*nbuffery[i]);
			i++;
		}
		i+=2;
	}
}


void GC_Spherical()
{
	float qwe;
	float ro, phi, theta;

		i=0;
		float d2r4=d2r*4.0f;

		for (theta=0.0f; theta<360.0f; theta+=(360.0f/spx))
		{
		  float ro0 = sin(((float)SDL_GetTicks()+theta*32.0f)/d2r4)*4.0f + 40.0f;
		  float cth = cos(theta/d2r);
		  float sth = sin(theta/d2r);
			for (phi=0.0f; phi<180.0f; phi+=(180.0f/spy))
			{
				ro = ro0 + sin(((float)SDL_GetTicks()+phi*32.0f)/d2r4)*8.0f;

				sphx[i]= ro * sin(phi/d2r) * cth * (sin(SDL_GetTicks()/276.0f)*0.3f+1.0f);
				sphy[i]= ro * sin(phi/d2r) * sth * (sin(SDL_GetTicks()/376.0f)*0.2f+1.0f);
				sphz[i]= ro * cos(phi/d2r) * (sin(SDL_GetTicks()/176.0f)*0.1f+1.0f);

				sphr[i]=sphx[i]+128+sin((3*sphx[i]+sphy[(int)(i+(float)SDL_GetTicks()/64.0f)])/12.0f)*64;
				sphg[i]=sphy[i]+128+sin((sphx[(int)(i+(float)SDL_GetTicks()/32.0f)]+2*sphy[i])/8.0f)*64;
				sphb[i++]=sphx[i]+sin((sphx[i]+3*sphy[i])/16.0f)*32+sin((4*sphx[i]+2*sphy[i])/8.0f)*32+128;
			}
		}
}
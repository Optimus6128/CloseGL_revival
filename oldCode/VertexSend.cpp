#pragma warning(disable: 4244)
#pragma warning(disable: 4809)
#pragma warning(disable: 4305)

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glaux.h>

#include <math.h>
#include <SDL.h>

#include "GridCalcs.h"
#include "precalcs.h"
#include "VertexSend.h"
#include "Parts.h"

const int nqx=128;
const int nqy=96; //nqx/ratio; (An einai dynaton!)

extern float gridx[nqx*nqy];
extern float gridy[nqx*nqy];
extern float gridz[nqx*nqy];

extern char fconv[256];


// === Plasma variables ===

extern unsigned char gridr[nqx*nqy];
extern unsigned char gridg[nqx*nqy];
extern unsigned char gridb[nqx*nqy];

extern unsigned char plgridr[pqx*pqy];
extern unsigned char plgridg[pqx*pqy];
extern unsigned char plgridb[pqx*pqy];
extern float plgrida[pqx*pqy];

// === Polar variables ===

extern unsigned char pgridr[gqx*gqy];
extern unsigned char pgridg[gqx*gqy];
extern unsigned char pgridb[gqx*gqy];
extern float pgrida[gqx*gqy];

extern bool wire;
extern GLuint texture[1];

extern int i,j,x,y,z;

extern float gx[16][gqx*gqy];
extern float gy[16][gqx*gqy];
extern float gz[16][gqx*gqy];

bool side[6];

// === 2d stars ===

float starx[1024], stary[1024], starz[1024], starspeed[1024];
unsigned char starcolr[1024], starcolg[1024], starcolb[1024];

extern star3d star[1024];
extern flower shape[32];

// ==== Object Test ====

extern GLfloat LightAmbient[];
extern GLfloat LightDiffuse[];
extern GLfloat LightPosition[];

extern float xo[16384],yo[16384],zo[16384];
extern int lp0[32768],lp1[32768];
extern int pp0[32768],pp1[32768],pp2[32768];
extern vector nv[32768];
extern vector pnv[16384];

extern int ndts,nlns,npls;

//int way=0,//nways=3;
extern int tex;

// === Water ====

extern float hbuffer[wqx*wqy];

extern float nbufferx[wqx*wqy];
extern float nbuffery[wqx*wqy];
extern float nbufferz[wqx*wqy];

// === Spherical ===

extern float sphx[spx*spy];
extern float sphy[spx*spy];
extern float sphz[spx*spy];

extern unsigned char sphr[spx*spy];
extern unsigned char sphg[spx*spy];
extern unsigned char sphb[spx*spy];


const float pi=3.14151693;
const float d2r=180.0/pi;

// -------------------------------------------------------------------------------------

// ======== FlatGrid ========


void VS_SkyBox()
{
	float sbsize=1024.0f;
	float ystretch=4.0f;

	glBindTexture(GL_TEXTURE_2D, texture[9]);

	glBegin(GL_QUADS);

	glColor3ub(255,255,255);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-sbsize, -sbsize, -sbsize*ystretch);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(sbsize, -sbsize, -sbsize*ystretch);
	glTexCoord2f(1.0f, 1.0f*ystretch);
	glVertex3f(sbsize, -sbsize, sbsize*ystretch);
	glTexCoord2f(0.0f, 1.0f*ystretch);
	glVertex3f(-sbsize, -sbsize, sbsize*ystretch);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-sbsize, sbsize, -sbsize*ystretch);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(sbsize, sbsize, -sbsize*ystretch);
	glTexCoord2f(1.0f, 1.0f*ystretch);
	glVertex3f(sbsize, sbsize, sbsize*ystretch);
	glTexCoord2f(0.0f, 1.0f*ystretch);
	glVertex3f(-sbsize, sbsize, sbsize*ystretch);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-sbsize, -sbsize, -sbsize*ystretch);
	glTexCoord2f(1.0f, 1.0f*ystretch);
	glVertex3f(-sbsize, -sbsize, sbsize*ystretch);
	glTexCoord2f(0.0f, 1.0f*ystretch);
	glVertex3f(-sbsize, sbsize, sbsize*ystretch);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-sbsize, sbsize, -sbsize*ystretch);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(sbsize, -sbsize, -sbsize*ystretch);
	glTexCoord2f(1.0f, 1.0f*ystretch);
	glVertex3f(sbsize, -sbsize, sbsize*ystretch);
	glTexCoord2f(0.0f, 1.0f*ystretch);
	glVertex3f(sbsize, sbsize, sbsize*ystretch);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(sbsize, sbsize, -sbsize*ystretch);

	glEnd();
}


float VS_FontWrite(char c, float xcp, float ycp, float z)
{
	float fsize=256.0f/z;
	float xt=(fconv[c]*16.0f)/1024.0f;
	float fd=16.0f/1024.0f;

	glBegin(GL_QUADS);
		glTexCoord2f(xt+fd, 1.0);
		glVertex3f(xcp,ycp,z);
		glTexCoord2f(xt, 1.0);
		glVertex3f(xcp+fsize,ycp,z);
		glTexCoord2f(xt, 0.0);
		glVertex3f(xcp+fsize,ycp+fsize,z);
		glTexCoord2f(xt+fd, 0.0f);
		glVertex3f(xcp,ycp+fsize,z);
	glEnd();

	return xcp-fsize;
}

void VS_TextWrite(char *text, float xtp, float ytp, float z)
{
	do{
        xtp=VS_FontWrite(*text++, xtp, ytp, z);
    }while(*(text)!=0);
}

void VS_Fade1(char fdc, float bsize)
{
	glBegin(GL_QUADS);
		glColor4ub(255.0f-fdc,255.0f-fdc,255.0f-fdc,0.0f);
		glVertex3f(-bsize,-bsize,-16.0f);
		glVertex3f(bsize,-bsize,-16.0f);
		glVertex3f(bsize,bsize,-16.0f);
		glVertex3f(-bsize,bsize,-16.0f);
	glEnd();
}

void VS_Fade2(char fdc, float bsize)
{
	glBegin(GL_QUADS);
		glColor4ub(fdc,fdc,fdc,fdc);
		glVertex3f(-bsize,-bsize,-16.0f);
		glVertex3f(bsize,-bsize,-16.0f);
		glVertex3f(bsize,bsize,-16.0f);
		glVertex3f(-bsize,bsize,-16.0f);
	glEnd();
}


void VS_FlatGrid()
{
	int xq,yq;

	glBegin(GL_QUADS);

			i=pqx+1;
			for (y=1; y<pqy-1; y+=2)
			{
				yq=y-(pqy>>1);
				for (x=1; x<pqx-1; x+=2)
				{
					xq=x-(pqx>>1);

					glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
					glVertex3f(xq,yq,plgrida[i]);

					glColor3ub(plgridr[i-1],plgridg[i-1],plgridb[i-1]);
					glVertex3f(xq-1,yq,plgrida[i-1]);

					glColor3ub(plgridr[i-1+pqx],plgridg[i-1+pqx],plgridb[i-1+pqx]);
					glVertex3f(xq-1,yq+1,plgrida[i-1+pqx]);

					glColor3ub(plgridr[i+pqx],plgridg[i+pqx],plgridb[i+pqx]);
					glVertex3f(xq,yq+1,plgrida[i+pqx]);


					glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
					glVertex3f(xq,yq,plgrida[i]);

					glColor3ub(plgridr[i+pqx],plgridg[i+pqx],plgridb[i+pqx]);
					glVertex3f(xq,yq+1,plgrida[i+pqx]);

					glColor3ub(plgridr[i+pqx+1],plgridg[i+pqx+1],plgridb[i+pqx+1]);
					glVertex3f(xq+1,yq+1,plgrida[i+pqx+1]);

					glColor3ub(plgridr[i+1],plgridg[i+1],plgridb[i+1]);
					glVertex3f(xq+1,yq,plgrida[i+1]);



					glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
					glVertex3f(xq,yq,plgrida[i]);

					glColor3ub(plgridr[i+1],plgridg[i+1],plgridb[i+1]);
					glVertex3f(xq+1,yq,plgrida[i+1]);

					glColor3ub(plgridr[i+1-pqx],plgridg[i+1-pqx],plgridb[i+1-pqx]);
					glVertex3f(xq+1,yq-1,plgrida[i+1-pqx]);

					glColor3ub(plgridr[i-pqx],plgridg[i-pqx],plgridb[i-pqx]);
					glVertex3f(xq,yq-1,plgrida[i-pqx]);


					glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
					glVertex3f(xq,yq,plgrida[i]);

					glColor3ub(plgridr[i-pqx],plgridg[i-pqx],plgridb[i-pqx]);
					glVertex3f(xq,yq-1,plgrida[i-pqx]);

					glColor3ub(plgridr[i-pqx-1],plgridg[i-pqx-1],plgridb[i-pqx-1]);
					glVertex3f(xq-1,yq-1,plgrida[i-pqx-1]);

					glColor3ub(plgridr[i-1],plgridg[i-1],plgridb[i-1]);
					glVertex3f(xq-1,yq,plgrida[i-1]);


					i+=2;
				}
				i+=pqx+2;
				
			}


		glEnd();
}



void VS_FlatGridNew(int face)
{
	float *pgx, *pgy, *pgz;

	pgx=&gx[face][1+gqx];
	pgy=&gy[face][1+gqx];
	pgz=&gz[face][1+gqx];

	glBegin(GL_QUADS);


			i=1+gqx;
			for (y=1; y<gqy-1; y+=2)
			{
				for (x=1; x<gqx-1; x+=2)
				{

					glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
					glVertex3f(*pgx,*pgy,*pgz);

					glColor3ub(pgridr[i-1],pgridg[i-1],pgridb[i-1]);
					glVertex3f(*(pgx-1),*(pgy-1),*(pgz-1));

					glColor3ub(pgridr[i-1+gqx],pgridg[i-1+gqx],pgridb[i-1+gqx]);
					glVertex3f(*(pgx-1+gqx),*(pgy-1+gqx),*(pgz-1+gqx));

					glColor3ub(pgridr[i+gqx],pgridg[i+gqx],pgridb[i+gqx]);
					glVertex3f(*(pgx+gqx),*(pgy+gqx),*(pgz+gqx));


					glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
					glVertex3f(*pgx,*pgy,*pgz);

					glColor3ub(pgridr[i+gqx],pgridg[i+gqx],pgridb[i+gqx]);
					glVertex3f(*(pgx+gqx),*(pgy+gqx),*(pgz+gqx));

					glColor3ub(pgridr[i+gqx+1],pgridg[i+gqx+1],pgridb[i+gqx+1]);
					glVertex3f(*(pgx+1+gqx),*(pgy+1+gqx),*(pgz+1+gqx));

					glColor3ub(pgridr[i+1],pgridg[i+1],pgridb[i+1]);
					glVertex3f(*(pgx+1),*(pgy+1),*(pgz+1));



					glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
					glVertex3f(*pgx,*pgy,*pgz);

					glColor3ub(pgridr[i+1],pgridg[i+1],pgridb[i+1]);
					glVertex3f(*(pgx+1),*(pgy+1),*(pgz+1));

					glColor3ub(pgridr[i+1-gqx],pgridg[i+1-gqx],pgridb[i+1-gqx]);
					glVertex3f(*(pgx+1-gqx),*(pgy+1-gqx),*(pgz+1-gqx));

					glColor3ub(pgridr[i-gqx],pgridg[i-gqx],pgridb[i-gqx]);
					glVertex3f(*(pgx-gqx),*(pgy-gqx),*(pgz-gqx));


					glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
					glVertex3f(*pgx,*pgy,*pgz);

					glColor3ub(pgridr[i-gqx],pgridg[i-gqx],pgridb[i-gqx]);
					glVertex3f(*(pgx-gqx),*(pgy-gqx),*(pgz-gqx));

					glColor3ub(pgridr[i-gqx-1],pgridg[i-gqx-1],pgridb[i-gqx-1]);
					glVertex3f(*(pgx-1-gqx),*(pgy-1-gqx),*(pgz-1-gqx));

					glColor3ub(pgridr[i-1],pgridg[i-1],pgridb[i-1]);
					glVertex3f(*(pgx-1),*(pgy-1),*(pgz-1));


					i+=2;
					pgx+=2; pgy+=2; pgz+=2;
				}
				i+=(gqx+2);
				pgx+=(gqx+2); pgy+=(gqx+2); pgz+=(gqx+2);
			}


		glEnd();
}


void VS_Floor(float y, float xsize, float zsize, float sdiv)
{
	float x,z;
	float dx=xsize/sdiv;
	float dz=zsize/sdiv;

	float x0=-xsize/2.0f;
	float x1=xsize/2.0f;
	float z0=0;
	float z1=zsize;

	float f=0.1f;

	float speed=SDL_GetTicks()/512.0f;

	glBindTexture(GL_TEXTURE_2D, texture[2]);

 	glBegin(GL_QUADS);
	
		for (z=z0; z<z1; z+=dz)
			for (x=x0; x<x1; x+=dx)
			{
				glColor3f(1-z/zsize, 1-z/zsize, 1-z/zsize);

				glTexCoord2f(x/(xsize*f),z/(zsize*f)+speed);
				glVertex3f(x,y,-z);

				glTexCoord2f((x+dx)/(xsize*f),z/(zsize*f)+speed);
				glVertex3f(x+dx,y,-z);

				glTexCoord2f((x+dx)/(xsize*f),(z+dz)/(zsize*f)+speed);
				glVertex3f(x+dx,y,-z-dz);

				glTexCoord2f(x/(xsize*f),(z+dz)/(zsize*f)+speed);
				glVertex3f(x,y,-z-dz);
			}
	glEnd();
}


void VS_Stars2d(float distance, float size, int i0, int i1)
{
	int i;

	for (i=i0; i<i1; i++)
	{
		starx[i]+=starspeed[i];
		if (starx[i]<-1024.0f) starx[i]=1024.0f;
		VS_Blob(starx[i], stary[i], starz[i]+distance, starcolr[i], starcolg[i], starcolb[i], size);
	}
}


// ======== Distort ========

void VS_Distort()
{

	float tpx=0, tpy=0;
	float dpx=1.0/nqx, dpy=1.0/nqy;

	switch (wire)
	{
	case false:

		glBindTexture(GL_TEXTURE_2D, texture[0]);

		glBegin(GL_QUADS);

			i=0;
			for (y=0; y<nqy-1; y++)
			{
				tpx=0;

				for (x=0; x<nqx-1; x++)
				{
					glTexCoord2f(tpx,tpy);
					glVertex3f(gridx[i],gridy[i],gridz[i]);
					glColor3ub(gridr[i],gridg[i],gridb[i]);

					glTexCoord2f(tpx+dpx,tpy);
					glVertex3f(gridx[i+1],gridy[i+1],gridz[i+1]);
					glColor3ub(gridr[i+1],gridg[i+1],gridb[i+1]);

					glTexCoord2f(tpx+dpx,tpy+dpy);
					glVertex3f(gridx[i+1+nqx],gridy[i+1+nqx],gridz[i+1+nqx]);
					glColor3ub(gridr[i+1+nqx],gridg[i+1+nqx],gridb[i+1+nqx]);

					glTexCoord2f(tpx,tpy+dpy);
					glVertex3f(gridx[i+nqx],gridy[i+nqx],gridz[i+nqx]);
					glColor3ub(gridr[i+nqx],gridg[i+nqx],gridb[i+nqx]);


					i++;
					tpx+=dpx;
				}
				i++;
				tpy+=dpy;
			}

		glEnd();
	break;

	case true:

		glBindTexture(GL_TEXTURE_2D, texture[0]);

		glBegin(GL_LINES);

		glColor3ub(255,255,255);

			i=0;
			for (y=0; y<nqy-1; y++)
			{
				tpx=0;

				for (x=0; x<nqx-1; x++)
				{
					glTexCoord2f(tpx,tpy);
					glVertex3d(gridx[i],gridy[i],gridz[i]);

					glTexCoord2f(tpx+dpx,tpy);
					glVertex3d(gridx[i+1],gridy[i+1],gridz[i+1]);

					glTexCoord2f(tpx+dpx,tpy);
					glVertex3d(gridx[i+1],gridy[i+1],gridz[i+1]);

					glTexCoord2f(tpx+dpx,tpy+dpy);
					glVertex3d(gridx[i+1+nqx],gridy[i+1+nqx],gridz[i+1+nqx]);

					glTexCoord2f(tpx+dpx,tpy+dpy);
					glVertex3d(gridx[i+1+nqx],gridy[i+1+nqx],gridz[i+1+nqx]);

					glTexCoord2f(tpx,tpy+dpy);
					glVertex3d(gridx[i+nqx],gridy[i+nqx],gridz[i+nqx]);

					glTexCoord2f(tpx,tpy+dpy);
					glVertex3d(gridx[i+nqx],gridy[i+nqx],gridz[i+nqx]);

					glTexCoord2f(tpx,tpy);
					glVertex3d(gridx[i],gridy[i],gridz[i]);

					i++;
					tpx+=dpx;
				}
				i++;
				tpy+=dpy;
			}

		glEnd();
	break;

	default:
		break;
	}
}



// ======== GridPoints ========





/*
void VS_GridPoints3D()
{

	float npts=8;
	float hx=32;

	p[0].x=-hx; p[0].y=hx; p[0].z=-hx;
	p[1].x=hx; p[1].y=hx; p[1].z=-hx;

	p[2].x=hx; p[2].y=-hx; p[2].z=-hx;
	p[3].x=-hx; p[3].y=-hx; p[3].z=-hx;

	p[4].x=-hx; p[4].y=hx; p[4].z=hx;
	p[5].x=hx; p[5].y=hx; p[5].z=hx;

	p[6].x=hx; p[6].y=-hx; p[6].z=hx;
	p[7].x=-hx; p[7].y=-hx; p[7].z=hx;

	glBegin(GL_POINTS);

	glColor3f(1.0,0.0,0.0);


	for (z=0; z<16; z++)
		for (y=0; y<16; y++)
			for (x=0; x<16; x++)
				for (i=0; i<8; i++)
					glVertex3f(p[i].x+x/32.0f,p[i].y+y/32.0f,p[i].z+z/32.0f);


	glColor3f(1.0,1.0,1.0);

	float x0,y0,z0;
	float dx,dy,dz,dx0,dy0,dz0;
	float ddx,ddy,ddz;
	float yp0,dyp0;
	float zp0,dzp0;

	dx=(p[1].x-p[0].x)/(npts-1);
	dy0=(p[3].y-p[0].y)/(npts-1);
	dz=(p[4].z-p[0].z)/(npts-1);

	dx0=(p[3].x-p[0].x)/(npts-1);
	dyp0=dy=(p[1].y-p[0].y)/(npts-1);

	ddx=((p[2].x-p[3].x) - (p[1].x-p[0].x))/((npts-1)*(npts-1));
	ddy=((p[2].y-p[3].y) - (p[1].y-p[0].y))/((npts-1)*(npts-1));
	ddz=((p[2].z-p[3].z) - (p[1].z-p[0].z))/((npts-1)*(npts-1));

	x0=p[0].x;
	z0=p[0].z;

	i=0;
	for (z=0; z<npts; z++)
	{
		yp0=p[0].y;
		for (y=0; y<npts; y++)
		{
			x0=p[0].x + y*dx0;
			y0=yp0;
			for (x=0; x<npts; x++)
			{
				glVertex3f(x0,y0,z0);
				x0+=dx;
				y0+=dy;
			}
			dx+=ddx;
			dy+=ddy;
			yp0+=dy0;
		}
		z0+=dz;
	}

	glEnd();
}
*/




void VS_ObjectShow(int way)
{

	switch(way)
	{

		case 0:
			glBegin(GL_POINTS);
			for (i=0; i<ndts; i++)
				glVertex3f(xo[i],yo[i],zo[i]);
			glEnd();
		break;


		case 1:
			/*
			glBegin(GL_LINES);
			for (i=0; i<nlns; i++)
			{
				glVertex3f(xo[lp0[i]],yo[lp0[i]],zo[lp0[i]]);
				glVertex3f(xo[lp1[i]],yo[lp1[i]],zo[lp1[i]]);
			}
			glEnd();
			*/
		break;


		case 2:
			glBegin(GL_TRIANGLES);
			for (i=0; i<npls; i++)
			{

				glNormal3f(pnv[pp0[i]].x,pnv[pp0[i]].y,pnv[pp0[i]].z);
				glVertex3f(xo[pp0[i]],yo[pp0[i]],zo[pp0[i]]);

				glNormal3f(pnv[pp1[i]].x,pnv[pp1[i]].y,pnv[pp1[i]].z);
				glVertex3f(xo[pp1[i]],yo[pp1[i]],zo[pp1[i]]);

				glNormal3f(pnv[pp2[i]].x,pnv[pp2[i]].y,pnv[pp2[i]].z);
				glVertex3f(xo[pp2[i]],yo[pp2[i]],zo[pp2[i]]);
			}
			glEnd();
		break;

		default:
			break;
	}

//	RenderLight();
}


void RenderLight()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	VS_Blob(LightPosition[0], LightPosition[1], LightPosition[2],255,255,255,32.0f);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}


void VS_Stars3d()
{
	float mulc;

	glBegin(GL_POINTS);
	for (i=0; i<1024; i++)
	{
		mulc=(256.0f+star[i].z)/256.0f;
		glPointSize(star[i].size);
		glColor3ub(star[i].rcol * mulc, star[i].gcol * mulc, star[i].bcol * mulc);
		glVertex3f(star[i].x, star[i].y, star[i].z);
	}
	glEnd();
}


void VS_Flower(flower flo)
{
	float ro,theta;
	float theta2ub=360.0f/256.0f;
	float thk=SDL_GetTicks()/128.0f;
	float fx,fy,fz;

	float mulc = (256.0f+flo.zfp)/256.0f;

	glPointSize(2);
	float flthetastep=1.0f;

	glBegin(GL_POINTS);
	for (theta=0; theta<360; theta+=flthetastep)
	{
		ro=sin((theta + flo.tmul1 * thk)/(d2r/flo.angdiv1)) * flo.sinmul1 + cos((theta + flo.tmul2 * thk)/(d2r/flo.angdiv2)) * flo.sinmul2 + flo.fsize;
		fx=ro*cos(theta/d2r) + flo.xfp;
		fy=ro*sin(theta/d2r) + flo.yfp;
		fz=flo.zfp;
		glColor3ub((theta/(theta2ub*flo.rdiv))*mulc, (theta/(theta2ub*flo.gdiv))*mulc, (theta/(theta2ub*flo.bdiv))*mulc);
		glVertex3f(fx,fy,fz);
	}
	glEnd();
}


void VS_Blob(float x, float y, float z, unsigned char r, unsigned char g, unsigned char b, float bsize)
{
	int k=(int)(sin(SDL_GetTicks()/64.0f)*4.0f+5.0f);
	k=1;

		glBindTexture(GL_TEXTURE_2D, texture[6]);

		glColor3ub(r,g,b);

		glBegin(GL_QUADS);

			for (i=0; i<k; i++)
			{
					glTexCoord2f(0,0);
					glVertex3f(-bsize+x,-bsize+y,z);

					glTexCoord2f(1,0);
					glVertex3f(bsize+x,-bsize+y,z);
		
					glTexCoord2f(1,1);
					glVertex3f(bsize+x,bsize+y,z);

					glTexCoord2f(0,1);
					glVertex3f(-bsize+x,bsize+y,z);
			}

		glEnd();
}


void VS_Water(int texn, float px, float py)
{
	int xq,yq;

	float tpx, tpy;
	float dpx=2.0f/wqx, dpy=2.0f/wqy;



	glBindTexture(GL_TEXTURE_2D, texture[texn]);

	glBegin(GL_QUADS);

			i=wqx+1;
			tpy=0.0f+py;
			for (y=1; y<wqy-1; y++)
			{
				tpx=0.0f+px;
				yq=y-(wqy>>1);
				for (x=1; x<wqx-1; x++)
				{
					xq=x-(wqx>>1);

					glTexCoord2f(tpx,tpy);
					glNormal3f(nbufferx[i],nbuffery[i],nbufferz[i]);
					glVertex3f(xq,yq,hbuffer[i]);

					glTexCoord2f(tpx+dpx,tpy);
					glNormal3f(nbufferx[i+1],nbuffery[i+1],nbufferz[i+1]);
					glVertex3f(xq+1,yq,hbuffer[i+1]);

					glTexCoord2f(tpx+dpx,tpy+dpy);
					glNormal3f(nbufferx[i+1+wqx],nbuffery[i+1+wqx],nbufferz[i+1+wqx]);
					glVertex3f(xq+1,yq+1,hbuffer[i+1+wqx]);

					glTexCoord2f(tpx,tpy+dpy);
					glNormal3f(nbufferx[i+wqx],nbuffery[i+wqx],nbufferz[i+wqx]);
					glVertex3f(xq,yq+1,hbuffer[i+wqx]);

					tpx+=dpx;
					i++;
				}
				i+=2;
				tpy+=dpy;
			}
		glEnd();

//	RenderLight();
}


void VS_Tile(float xpos, float ypos, float blend)
{
	float xsize=4.0f, ysize=3.0f;
	float z=-15.0f;

	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, blend);
	glBegin(GL_QUADS);
		glVertex3f(xpos, ypos, z);
		glVertex3f(xpos+xsize, ypos, z);
		glVertex3f(xpos+xsize, ypos+ysize, z);
		glVertex3f(xpos, ypos+ysize, z);
	glEnd();
	glEnable(GL_LIGHTING);
}


void VS_PisinaGrid(float xsize, float ysize, float xpos, float ypos, float tscale, float height)
{

	float plght=0.25f;
	float pdrkn=0.0f;

	glBegin(GL_QUADS);
		glNormal3f(0.0f,0.0f,0.5f);
		glTexCoord2f(0.0f,tscale);
			glVertex3f(0.0f + xpos, 0.0f + ypos, height);
		glTexCoord2f(0.0f,0.0f);
			glVertex3f(xsize + xpos, 0.0f + ypos, height);
		glTexCoord2f(tscale,0.0f);
			glVertex3f(xsize + xpos, ysize + ypos, height);
		glTexCoord2f(tscale,tscale);
			glVertex3f(0.0f + xpos, ysize + ypos, height);
	glEnd();
}


void VS_Pisina(float height1, float height2)
{
	float psqx=(wqx-2)/2.1f;
	float psqy=(wqy-2)/2.1f;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 8);

	glColor3f(1.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx, psqy, height1);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(psqx, psqy, height2);
		glTexCoord2f(4.0f,1.0f);
		glVertex3f(psqx, -psqy, height2);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(psqx, -psqy, height1);
	glEnd();

	glBegin(GL_QUADS);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx, psqy, height1);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(psqx, psqy, height2);
		glTexCoord2f(4.0f,1.0f);
		glVertex3f(-psqx, psqy, height2);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(-psqx, psqy, height1);
	glEnd();

	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-psqx, psqy, height1);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-psqx, psqy, height2);
		glTexCoord2f(4.0f,1.0f);
		glVertex3f(-psqx, -psqy, height2);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(-psqx, -psqy, height1);
	glEnd();

	glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx, -psqy, height1);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(psqx, -psqy, height2);
		glTexCoord2f(4.0f,1.0f);
		glVertex3f(-psqx, -psqy, height2);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(-psqx, -psqy, height1);
	glEnd();


	// Box

	glBindTexture(GL_TEXTURE_2D, 11);
	float height3=height1;
	float height4=height3+256;
	float bxscale=3.0f;

	glColor3f(1.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx*bxscale, psqy*bxscale, height3);
		glTexCoord2f(0.0f,2.0f);
		glVertex3f(psqx*bxscale, psqy*bxscale, height4);
		glTexCoord2f(4.0f,2.0f);
		glVertex3f(psqx*bxscale, -psqy*bxscale, height4);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(psqx*bxscale, -psqy*bxscale, height3);
	glEnd();

	glBegin(GL_QUADS);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx*bxscale, psqy*bxscale, height3);
		glTexCoord2f(0.0f,2.0f);
		glVertex3f(psqx*bxscale, psqy*bxscale, height4);
		glTexCoord2f(4.0f,2.0f);
		glVertex3f(-psqx*bxscale, psqy*bxscale, height4);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(-psqx*bxscale, psqy*bxscale, height3);
	glEnd();

	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-psqx*bxscale, psqy*bxscale, height3);
		glTexCoord2f(0.0f,2.0f);
		glVertex3f(-psqx*bxscale, psqy*bxscale, height4);
		glTexCoord2f(4.0f,2.0f);
		glVertex3f(-psqx*bxscale, -psqy*bxscale, height4);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(-psqx*bxscale, -psqy*bxscale, height3);
	glEnd();

	glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx*bxscale, -psqy*bxscale, height3);
		glTexCoord2f(0.0f,2.0f);
		glVertex3f(psqx*bxscale, -psqy*bxscale, height4);
		glTexCoord2f(4.0f,2.0f);
		glVertex3f(-psqx*bxscale, -psqy*bxscale, height4);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(-psqx*bxscale, -psqy*bxscale, height3);
	glEnd();


	float wtrx=SDL_GetTicks()/12288.0f;
	float wtry=SDL_GetTicks()/-18432.0f;

	glEnable(GL_TEXTURE_2D);
	VS_WannabeCaustics(2, wtrx, wtry, height2);


	glBindTexture(GL_TEXTURE_2D, 9);

	float pln=4.0f;

	VS_PisinaGrid(psqx*2, psqy*2, psqx, -psqy, 4.0f, height1);
	VS_PisinaGrid(psqx*2, psqy*2, psqx, psqy, 4.0f, height1);
	VS_PisinaGrid(psqx*2, psqy*2, psqx, -3*psqy, 4.0f, height1);

	VS_PisinaGrid(psqx*2, psqy*2, -3*psqx, -psqy, 4.0f, height1);
	VS_PisinaGrid(psqx*2, psqy*2, -3*psqx, psqy, 4.0f, height1);
	VS_PisinaGrid(psqx*2, psqy*2, -3*psqx, -3*psqy, 4.0f, height1);

	VS_PisinaGrid(psqx*2, psqy*2, -psqx, psqy, 4.0f, height1);
	VS_PisinaGrid(psqx*2, psqy*2, -psqx, -3*psqy, 4.0f, height1);

}


void VS_WannabeCaustics(int texn, float px, float py, float hgt)
{
	int xq,yq;
	int jumps=4;
	int shit=wqx/jumps;
	float ficken=4.0f;

	float tpx, tpy;
	float dpx=(float)jumps*ficken/wqx, dpy=float(jumps)*ficken/wqy;

	float dispx[wqx*wqy];
	float dispy[wqx*wqy];

		i=wqx+1;
		tpy=0.0f+py;
		for (y=0; y<wqy; y+=jumps)
		{
			tpx=0.0f+px;
			yq=y-(wqy>>1);
			for (x=0; x<wqx; x+=jumps)
			{
				xq=x-(wqx>>1);

				dispx[i]=tpx+sin((y+SDL_GetTicks()/128.0f)/4.0f)*0.03f;
				dispy[i]=tpy+sin((x+SDL_GetTicks()/192.0f)/4.0f)*0.05f;

				tpx+=dpx;
				i++;
			}
			tpy+=dpy;
		}


	glBindTexture(GL_TEXTURE_2D, texture[texn]);

	glBegin(GL_QUADS);

			i=wqx+1;
			tpy=0.0f+py;
			for (y=1; y<wqy-1; y+=jumps)
			{
				yq=y-(wqy>>1);
				for (x=1; x<wqx-1; x+=jumps)
				{
					xq=x-(wqx>>1);

					glTexCoord2f(dispx[i],dispy[i]);
					glVertex3f(xq,yq,hgt);

					glTexCoord2f(dispx[i+1],dispy[i+1]);
					glVertex3f(xq+jumps,yq,hgt);

					glTexCoord2f(dispx[i+shit+1],dispy[i+shit+1]);
					glVertex3f(xq+jumps,yq+jumps,hgt);

					glTexCoord2f(dispx[i+shit],dispy[i+shit]);
					glVertex3f(xq,yq+jumps,hgt);

					i++;
				}
			}
		glEnd();

}


void VS_Spherical()
{
	int xq,yq;

	float tpx, tpy;
	float dpx=1.0f/wqx, dpy=1.0f/wqy;


	glBindTexture(GL_TEXTURE_2D, texture[3]);

	glBegin(GL_QUADS);

			tpy=0.0f;
			i=spx+1;
			for (y=1; y<spy-1; y+=2)
			{

				tpx=0.0f;
				yq=y-(wqy>>1);
				for (x=1; x<spx-1; x+=2)
				{
					xq=x-(wqx>>1);

					glTexCoord2f(tpx,tpy);
					glColor3ub(sphr[i],sphg[i],sphb[i]);
					glVertex3f(sphx[i],sphy[i],sphz[i]);

					glTexCoord2f(tpx-dpx,tpy);
					glColor3ub(sphr[i-1],sphg[i-1],sphb[i-1]);
					glVertex3f(sphx[i-1],sphy[i-1],sphz[i-1]);

					glTexCoord2f(tpx-dpx,tpy+dpy);
					glColor3ub(sphr[i-1+spx],sphg[i-1+spx],sphb[i-1+spx]);
					glVertex3f(sphx[i-1+spx],sphy[i-1+spx],sphz[i-1+spx]);

					glTexCoord2f(tpx,tpy+dpy);
					glColor3ub(sphr[i+spx],sphg[i+spx],sphb[i+spx]);
					glVertex3f(sphx[i+spx],sphy[i+spx],sphz[i+spx]);


					glTexCoord2f(tpx,tpy);
					glColor3ub(sphr[i],sphg[i],sphb[i]);
					glVertex3f(sphx[i],sphy[i],sphz[i]);

					glTexCoord2f(tpx,tpy+dpy);
					glColor3ub(sphr[i+spx],sphg[i+spx],sphb[i+spx]);
					glVertex3f(sphx[i+spx],sphy[i+spx],sphz[i+spx]);

					glTexCoord2f(tpx+dpx,tpy+dpy);
					glColor3ub(sphr[i+spx+1],sphg[i+spx+1],sphb[i+spx+1]);
					glVertex3f(sphx[i+spx+1],sphy[i+spx+1],sphz[i+spx+1]);

					glTexCoord2f(tpx+dpx,tpy);
					glColor3ub(sphr[i+1],sphg[i+1],sphb[i+1]);
					glVertex3f(sphx[i+1],sphy[i+1],sphz[i+1]);


					glTexCoord2f(tpx,tpy);
					glColor3ub(sphr[i],sphg[i],sphb[i]);
					glVertex3f(sphx[i],sphy[i],sphz[i]);

					glTexCoord2f(tpx+dpx,tpy);
					glColor3ub(sphr[i+1],sphg[i+1],sphb[i+1]);
					glVertex3f(sphx[i+1],sphy[i+1],sphz[i+1]);

					glTexCoord2f(tpx+dpx,tpy-dpy);
					glColor3ub(sphr[i+1-spx],sphg[i+1-spx],sphb[i+1-spx]);
					glVertex3f(sphx[i+1-spx],sphy[i+1-spx],sphz[i+1-spx]);

					glTexCoord2f(tpx,tpy-dpy);
					glColor3ub(sphr[i-spx],sphg[i-spx],sphb[i-spx]);
					glVertex3f(sphx[i-spx],sphy[i-spx],sphz[i-spx]);


					glTexCoord2f(tpx,tpy);
					glColor3ub(sphr[i],sphg[i],sphb[i]);
					glVertex3f(sphx[i],sphy[i],sphz[i]);

					glTexCoord2f(tpx,tpy-dpy);
					glColor3ub(sphr[i-spx],sphg[i-spx],sphb[i-spx]);
					glVertex3f(sphx[i-spx],sphy[i-spx],sphz[i-spx]);

					glTexCoord2f(tpx-dpx,tpy-dpy);
					glColor3ub(sphr[i-spx-1],sphg[i-spx-1],sphb[i-spx-1]);
					glVertex3f(sphx[i-spx-1],sphy[i-spx-1],sphz[i-spx-1]);

					glTexCoord2f(tpx-dpx,tpy);
					glColor3ub(sphr[i-1],sphg[i-1],sphb[i-1]);
					glVertex3f(sphx[i-1],sphy[i-1],sphz[i-1]);

					tpx+=(dpx*2);
					i+=2;
				}
				tpy+=(dpy*2);
				i+=spx+2;
				
			}

	// correction

			tpx=0.0f;
				j=(spy-1)*spx+1; i=1;
				for (x=1; x<spx-1; x+=2)
				{

					glTexCoord2f(tpx,tpy-dpy);
					glColor3ub(sphr[j-spx],sphg[j-spx],sphb[j-spx]);
					glVertex3f(sphx[j-spx],sphy[j-spx],sphz[j-spx]);

					glTexCoord2f(tpx+dpx,tpy-dpy);
					glColor3ub(sphr[j-spx+1],sphg[j-spx+1],sphb[j-spx+1]);
					glVertex3f(sphx[j-spx+1],sphy[j-spx+1],sphz[j-spx+1]);

					glTexCoord2f(tpx+dpx,tpy);
					glColor3ub(sphr[j+1],sphg[j+1],sphb[j+1]);
					glVertex3f(sphx[j+1],sphy[j+1],sphz[j+1]);

					glTexCoord2f(tpx,tpy);
					glColor3ub(sphr[j],sphg[j],sphb[j]);
					glVertex3f(sphx[j],sphy[j],sphz[j]);


					glTexCoord2f(tpx,tpy-dpy);
					glColor3ub(sphr[j-spx],sphg[j-spx],sphb[j-spx]);
					glVertex3f(sphx[j-spx],sphy[j-spx],sphz[j-spx]);

					glTexCoord2f(tpx,tpy);
					glColor3ub(sphr[j],sphg[j],sphb[j]);
					glVertex3f(sphx[j],sphy[j],sphz[j]);

					glTexCoord2f(tpx-dpx,tpy);
					glColor3ub(sphr[j-1],sphg[j-1],sphb[j-1]);
					glVertex3f(sphx[j-1],sphy[j-1],sphz[j-1]);

					glTexCoord2f(tpx-dpx,tpy-dpy);
					glColor3ub(sphr[j-spx-1],sphg[j-spx-1],sphb[j-spx-1]);
					glVertex3f(sphx[j-spx-1],sphy[j-spx-1],sphz[j-spx-1]);


					glTexCoord2f(tpx,tpy);
					glColor3ub(sphr[j],sphg[j],sphb[j]);
					glVertex3f(sphx[j],sphy[j],sphz[j]);

					glTexCoord2f(tpx+dpx,tpy);
					glColor3ub(sphr[j+1],sphg[j+1],sphb[j+1]);
					glVertex3f(sphx[j+1],sphy[j+1],sphz[j+1]);

					glTexCoord2f(tpx+dpx,tpy+dpy);
					glColor3ub(sphr[i+1],sphg[i+1],sphb[i+1]);
					glVertex3f(sphx[i+1],sphy[i+1],sphz[i+1]);

					glTexCoord2f(tpx,tpy+dpy);
					glColor3ub(sphr[i],sphg[i],sphb[i]);
					glVertex3f(sphx[i],sphy[i],sphz[i]);


					glTexCoord2f(tpx,tpy);
					glColor3ub(sphr[j],sphg[j],sphb[j]);
					glVertex3f(sphx[j],sphy[j],sphz[j]);

					glTexCoord2f(tpx,tpy+dpy);
					glColor3ub(sphr[i],sphg[i],sphb[i]);
					glVertex3f(sphx[i],sphy[i],sphz[i]);

					glTexCoord2f(tpx-dpx,tpy+dpy);
					glColor3ub(sphr[i-1],sphg[i-1],sphb[i-1]);
					glVertex3f(sphx[i-1],sphy[i-1],sphz[i-1]);

					glTexCoord2f(tpx-dpx,tpy);
					glColor3ub(sphr[j-1],sphg[j-1],sphb[j-1]);
					glVertex3f(sphx[j-1],sphy[j-1],sphz[j-1]);

					tpx+=(dpx*2);
					i+=2;
					j+=2;
				}


		glEnd();

}


void VS_Tail(float sizex, int sdx, ssine s0, ssine s1, float r, float g, float b)
{
	const int n=256;
	float tail0[n], tail1[n];

	float dx=sizex/((float)(sdx+1));
	float zf=-256.0f;

	int i;
	for (i=0; i<sdx; i++)
	{
		tail0[i]=sin((i+SDL_GetTicks()/s0.speed)/s0.sinediv) * s0.sinemul + s0.sineadd;
		tail1[i]=sin((i+SDL_GetTicks()/s1.speed)/s1.sinediv) * s1.sinemul + s1.sineadd;
	}

	float x0=-sizex/2;

	glBegin(GL_QUADS);

	for (i=1; i<sdx; i++)
	{
		x0+=dx;
		float ix=(float)i/(float)sdx;
		glColor3f(ix*r, ix*g, ix*b);
		glVertex3f(x0, tail0[i-1], zf);
		glVertex3f(x0+dx, tail0[i], zf);
		glVertex3f(x0+dx, tail1[i], zf);
		glVertex3f(x0, tail1[i-1], zf);
	}

	glEnd();
}

void VS_CubeTest(point2d p0, point2d p1, point2d p2, point2d p3, float rcx, float rcy, float rcz, int cubeside)
{
	point2d pp0,pp1,pp2,pp3;

	pp0 = VS_RotatePoint(p0, rcx, -rcy, rcz);
	pp1 = VS_RotatePoint(p1, rcx, -rcy, rcz);
	pp2 = VS_RotatePoint(p2, rcx, -rcy, rcz);
	pp3 = VS_RotatePoint(p3, rcx, -rcy, rcz);

	vector view;
	view.x = 0.0f;
	view.y = 0.0f;
	view.z = -1.0f;

	vector side1, side2, sidenormal;

	side1.x = pp1.x - pp0.x;
	side1.y = pp1.y - pp0.y;
	side1.z = pp1.z - pp0.z;

	side2.x = pp3.x - pp0.x;
	side2.y = pp3.y - pp0.y;
	side2.z = pp3.z - pp0.z;

	sidenormal=CrossProduct(side1, side2);
	if (DotProduct(sidenormal,view)<0) side[cubeside]=true;
		else side[cubeside]=false;
	

}


point2d VS_RotatePoint(point2d p, float rpx, float rpy, float rpz)
{
	float cosxr=cos(rpx/d2r); float cosyr=cos(rpy/d2r); float coszr=cos(rpz/d2r);
	float sinxr=sin(rpx/d2r); float sinyr=sin(rpy/d2r); float sinzr=sin(rpz/d2r);

	float nx;
	point2d pp;

	pp.y = sinzr * p.x + coszr * p.y;
	pp.x = coszr * p.x - sinzr * p.y;

	pp.z = sinyr * pp.x + cosyr * p.z;
	pp.x = cosyr * pp.x - sinyr * p.z;

	nx = pp.y;
	pp.y = cosxr * nx - sinxr * pp.z;
	pp.z = sinxr * nx + cosxr * pp.z;


	return pp;
}

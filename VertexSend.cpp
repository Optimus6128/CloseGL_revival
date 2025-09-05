#pragma warning(disable: 4244)
#pragma warning(disable: 4809)
#pragma warning(disable: 4305)

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glaux.h>

#include <math.h>
#include <SDL/SDL.h>

#include "GridCalcs.h"
#include "precalcs.h"
#include "VertexSend.h"
#include "Parts.h"


#define PI 3.14151693f
#define D2R (180.0f / PI)

const int nqx=128;
const int nqy=96; //nqx/ratio; (An einai dynaton!)

extern float gridx[nqx*nqy];
extern float gridy[nqx*nqy];
extern float gridz[nqx*nqy];

extern char fconv[256];


// === Plasma variables ===

extern char gridr[nqx*nqy];
extern char gridg[nqx*nqy];
extern char gridb[nqx*nqy];

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
extern GLuint texture[9];

extern int globalTime;

extern float gx[16][gqx*gqy];
extern float gy[16][gqx*gqy];
extern float gz[16][gqx*gqy];

bool side[6];

// === 2d stars ===

float starx[1024], stary[1024], starz[1024], starspeed[1024];
char starcolr[1024], starcolg[1024], starcolb[1024];

extern star3d star[1024];
extern flower shape[32];

// ==== Object Test ====

extern GLfloat LightAmbient[];
extern GLfloat LightDiffuse[];
extern GLfloat LightPosition[];

extern float xo[16384],yo[16384],zo[16384];
extern int lp0[32768],lp1[32768];
extern int pp0[32768],pp1[32768],pp2[32768];
extern Vector nv[32768];
extern Vector pnv[16384];

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


// Tables to pass for Vertex Arrays
#define MAX_ARRAY_SIZE 65536

static float glArrayVertices[3 * MAX_ARRAY_SIZE];
static unsigned char glArrayColors3ub[3 * MAX_ARRAY_SIZE];
static float glArrayTexcoords[2 * MAX_ARRAY_SIZE];
static float glArrayNormals[3 * MAX_ARRAY_SIZE];

static float *glArrayVerticesPtr;
static unsigned char *glArrayColors3ubPtr;
static float *glArrayTexcoordsPtr;
static float *glArrayNormalsPtr;

#define M_glVertex3f(x,y,z) *glArrayVerticesPtr++ = (x); *glArrayVerticesPtr++ = (y); *glArrayVerticesPtr++ = (z);
#define M_glColor3ub(r,g,b) *glArrayColors3ubPtr++ = (r); *glArrayColors3ubPtr++ = (g); *glArrayColors3ubPtr++ = (b);
#define M_glTexCoord2f(u,v) *glArrayTexcoordsPtr++ = (u); *glArrayTexcoordsPtr++ = (v);
#define M_glNormal3f(x,y,z) *glArrayNormalsPtr++ = (x); *glArrayNormalsPtr++ = (y); *glArrayNormalsPtr++ = (z);

typedef struct VertexData
{
	Vector position;
	Color color;
	TexCoords texcoord;
	Vector normal;
} VertexData;

static VertexData *glArrayVertexDataPtr;
static VertexData glArrayVertexData[MAX_ARRAY_SIZE];

static unsigned short *glArrayIndexPtr;
static unsigned short glArrayIndices[MAX_ARRAY_SIZE];

#define N_glColor3ub(cr,cg,cb) glArrayVertexDataPtr->color.r = (cr); glArrayVertexDataPtr->color.g = (cg); glArrayVertexDataPtr->color.b = (cb);
#define N_glTexCoord2f(tu,tv) glArrayVertexDataPtr->texcoord.u = (tu); glArrayVertexDataPtr->texcoord.v = (tv);
#define N_glNormal3f(px,py,pz) glArrayVertexDataPtr->normal.x = (px); glArrayVertexDataPtr->normal.y = (py); glArrayVertexDataPtr->normal.z = (pz);
#define N_glVertex3f(px,py,pz) glArrayVertexDataPtr->position.x = (px); glArrayVertexDataPtr->position.y = (py); glArrayVertexDataPtr->position.z = (pz); ++glArrayVertexDataPtr;
#define N_glIndex4us(p0,p1,p2,p3) *glArrayIndexPtr++ = (p0); *glArrayIndexPtr++ = (p1); *glArrayIndexPtr++ = (p2); *glArrayIndexPtr++ = (p3);
#define N_glIndex2us(p0,p1) *glArrayIndexPtr++ = (p0); *glArrayIndexPtr++ = (p1);

// -------------------------------------------------------------------------------------

static void initGlArrayPointers()
{
	glArrayVerticesPtr = glArrayVertices;
	glArrayColors3ubPtr = glArrayColors3ub;
	glArrayTexcoordsPtr = glArrayTexcoords;
	glArrayNormalsPtr = glArrayNormals;
	glArrayVertexDataPtr = glArrayVertexData;
	glArrayIndexPtr = glArrayIndices;
}

static void renderVertexArrays(int count, bool hasColors, bool hasTexcoords, bool hasNormals, bool areTriangles = false)
{
	// Setup

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, glArrayVertices);

	if (hasColors) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_UNSIGNED_BYTE, 0, glArrayColors3ub);
	}

	if (hasNormals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, glArrayNormals);
	}

	if (hasTexcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, glArrayTexcoords);
	}


	// Draw
	
	if (areTriangles) {
		glDrawArrays(GL_TRIANGLES, 0, count);
	} else {
		glDrawArrays(GL_QUADS, 0, count);
	}


	// Clean up

	glDisableClientState(GL_VERTEX_ARRAY);

	if (hasColors) {
		glDisableClientState(GL_COLOR_ARRAY);
	}
	if (hasNormals) {
		glDisableClientState(GL_NORMAL_ARRAY);
	}
	if (hasTexcoords) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

static void renderVertexArraysPacked(int count, bool hasIndices, bool hasColors, bool hasTexcoords, bool hasNormals, bool areQuadStrips = true, bool areTriangles = false)
{
	// Setup

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(VertexData), &glArrayVertexData[0].position);

	if (hasColors) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(VertexData), &glArrayVertexData[0].color);
	}

	if (hasNormals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, sizeof(VertexData), &glArrayVertexData[0].normal);
	}

	if (hasTexcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData), &glArrayVertexData[0].texcoord);
	}


	// Draw
	
	if (hasIndices) {
		if (areTriangles) {
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, glArrayIndices);
		} else {
			if (areQuadStrips) {
				glDrawElements(GL_QUAD_STRIP, count, GL_UNSIGNED_SHORT, glArrayIndices);
			} else {
				glDrawElements(GL_QUADS, count, GL_UNSIGNED_SHORT, glArrayIndices);
			}
		}
	} else {
		if (areTriangles) {
			glDrawArrays(GL_TRIANGLES, 0, count);
		} else {
			if (areQuadStrips) {
				glDrawArrays(GL_QUAD_STRIP, 0, count);
			} else {
				glDrawArrays(GL_QUADS, 0, count);
			}
		}
	}


	// Clean up

	glDisableClientState(GL_VERTEX_ARRAY);

	if (hasColors) {
		glDisableClientState(GL_COLOR_ARRAY);
	}
	if (hasNormals) {
		glDisableClientState(GL_NORMAL_ARRAY);
	}
	if (hasTexcoords) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}


// ======== FlatGrid ========



float VS_FontWrite2(char c, float xcp, float ycp, float z)
{
	float st=0.1f;
	float fsize=(256.0f*st)/z;
	float xt=(fconv[c]*16.0f)/1024.0f;
	float fd=16.0f/1024.0f;

	glTexCoord2f(xt+fd, 1.0);
	glVertex3f(xcp,ycp,z);
	glTexCoord2f(xt, 1.0);
	glVertex3f(xcp+fsize,ycp,z);
	glTexCoord2f(xt, 0.0);
	glVertex3f(xcp+fsize,ycp+fsize,z);
	glTexCoord2f(xt+fd, 0.0f);
	glVertex3f(xcp,ycp+fsize,z);

	return xcp-fsize;
}

void VS_TextWrite2(char *text, float xtp, float ytp, float z)
{
	glBegin(GL_QUADS);

	do{
        xtp=VS_FontWrite2(*text++, xtp, ytp, z);
    }while(*(text)!=0);

	glEnd();
}


float VS_FontWrite(char c, float xcp, float ycp, float z)
{
	float fsize=256.0f/z;
	float xt=(fconv[c]*16.0f)/1024.0f;
	float fd=16.0f/1024.0f;

	glTexCoord2f(xt+fd, 1.0);
	glVertex3f(xcp,ycp,z);
	glTexCoord2f(xt, 1.0);
	glVertex3f(xcp+fsize,ycp,z);
	glTexCoord2f(xt, 0.0);
	glVertex3f(xcp+fsize,ycp+fsize,z);
	glTexCoord2f(xt+fd, 0.0f);
	glVertex3f(xcp,ycp+fsize,z);

	return xcp-fsize;
}

void VS_TextWrite(char *text, float xtp, float ytp, float z)
{
	glBegin(GL_QUADS);

	do{
        xtp=VS_FontWrite(*text++, xtp, ytp, z);
    }while(*(text)!=0);

	glEnd();
}



void VS_Fade1(unsigned char fdc, float bsize)
{
	glBegin(GL_QUADS);
		glColor4ub(255.0f-fdc,255.0f-fdc,255.0f-fdc,0.0f);
		glVertex3f(-bsize,-bsize,-4.0f);
		glVertex3f(bsize,-bsize,-4.0f);
		glVertex3f(bsize,bsize,-4.0f);
		glVertex3f(-bsize,bsize,-4.0f);
	glEnd();
}

void VS_Fade2(unsigned char fdc, float bsize)
{
	glBegin(GL_QUADS);
		glColor4ub(fdc,fdc,fdc,fdc);
		glVertex3f(-bsize,-bsize,-4.0f);
		glVertex3f(bsize,-bsize,-4.0f);
		glVertex3f(bsize,bsize,-4.0f);
		glVertex3f(-bsize,bsize,-4.0f);
	glEnd();
}


void VS_FlatGrid()
{
	int xq,yq;
	int count = 0;

	initGlArrayPointers();

	int i=pqx+1;
	for (int y=1; y<pqy-1; y+=2)
	{
		yq=y-(pqy>>1);
		for (int x=1; x<pqx-1; x+=2)
		{
			xq=x-(pqx>>1);

			M_glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
			M_glVertex3f(xq,yq,plgrida[i]);

			M_glColor3ub(plgridr[i-1],plgridg[i-1],plgridb[i-1]);
			M_glVertex3f(xq-1,yq,plgrida[i-1]);

			M_glColor3ub(plgridr[i-1+pqx],plgridg[i-1+pqx],plgridb[i-1+pqx]);
			M_glVertex3f(xq-1,yq+1,plgrida[i-1+pqx]);

			M_glColor3ub(plgridr[i+pqx],plgridg[i+pqx],plgridb[i+pqx]);
			M_glVertex3f(xq,yq+1,plgrida[i+pqx]);


			M_glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
			M_glVertex3f(xq,yq,plgrida[i]);

			M_glColor3ub(plgridr[i+pqx],plgridg[i+pqx],plgridb[i+pqx]);
			M_glVertex3f(xq,yq+1,plgrida[i+pqx]);

			M_glColor3ub(plgridr[i+pqx+1],plgridg[i+pqx+1],plgridb[i+pqx+1]);
			M_glVertex3f(xq+1,yq+1,plgrida[i+pqx+1]);

			M_glColor3ub(plgridr[i+1],plgridg[i+1],plgridb[i+1]);
			M_glVertex3f(xq+1,yq,plgrida[i+1]);



			M_glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
			M_glVertex3f(xq,yq,plgrida[i]);

			M_glColor3ub(plgridr[i+1],plgridg[i+1],plgridb[i+1]);
			M_glVertex3f(xq+1,yq,plgrida[i+1]);

			M_glColor3ub(plgridr[i+1-pqx],plgridg[i+1-pqx],plgridb[i+1-pqx]);
			M_glVertex3f(xq+1,yq-1,plgrida[i+1-pqx]);

			M_glColor3ub(plgridr[i-pqx],plgridg[i-pqx],plgridb[i-pqx]);
			M_glVertex3f(xq,yq-1,plgrida[i-pqx]);


			M_glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
			M_glVertex3f(xq,yq,plgrida[i]);

			M_glColor3ub(plgridr[i-pqx],plgridg[i-pqx],plgridb[i-pqx]);
			M_glVertex3f(xq,yq-1,plgrida[i-pqx]);

			M_glColor3ub(plgridr[i-pqx-1],plgridg[i-pqx-1],plgridb[i-pqx-1]);
			M_glVertex3f(xq-1,yq-1,plgrida[i-pqx-1]);

			M_glColor3ub(plgridr[i-1],plgridg[i-1],plgridb[i-1]);
			M_glVertex3f(xq-1,yq,plgrida[i-1]);

			count += 4;

			i+=2;
		}
		i+=pqx+2;
		
	}

	renderVertexArrays(4 * count, true, false, false);
}



void VS_FlatGridNew(int face)
{
	float *pgx, *pgy, *pgz;
	int count = 0;

	pgx=&gx[face][1+gqx];
	pgy=&gy[face][1+gqx];
	pgz=&gz[face][1+gqx];

	initGlArrayPointers();

	int i=1+gqx;
	for (int y=1; y<gqy-1; y+=2)
	{
		for (int x=1; x<gqx-1; x+=2)
		{

			M_glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
			M_glVertex3f(*pgx,*pgy,*pgz);

			M_glColor3ub(pgridr[i-1],pgridg[i-1],pgridb[i-1]);
			M_glVertex3f(*(pgx-1),*(pgy-1),*(pgz-1));

			M_glColor3ub(pgridr[i-1+gqx],pgridg[i-1+gqx],pgridb[i-1+gqx]);
			M_glVertex3f(*(pgx-1+gqx),*(pgy-1+gqx),*(pgz-1+gqx));

			M_glColor3ub(pgridr[i+gqx],pgridg[i+gqx],pgridb[i+gqx]);
			M_glVertex3f(*(pgx+gqx),*(pgy+gqx),*(pgz+gqx));


			M_glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
			M_glVertex3f(*pgx,*pgy,*pgz);

			M_glColor3ub(pgridr[i+gqx],pgridg[i+gqx],pgridb[i+gqx]);
			M_glVertex3f(*(pgx+gqx),*(pgy+gqx),*(pgz+gqx));

			M_glColor3ub(pgridr[i+gqx+1],pgridg[i+gqx+1],pgridb[i+gqx+1]);
			M_glVertex3f(*(pgx+1+gqx),*(pgy+1+gqx),*(pgz+1+gqx));

			M_glColor3ub(pgridr[i+1],pgridg[i+1],pgridb[i+1]);
			M_glVertex3f(*(pgx+1),*(pgy+1),*(pgz+1));



			M_glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
			M_glVertex3f(*pgx,*pgy,*pgz);

			M_glColor3ub(pgridr[i+1],pgridg[i+1],pgridb[i+1]);
			M_glVertex3f(*(pgx+1),*(pgy+1),*(pgz+1));

			M_glColor3ub(pgridr[i+1-gqx],pgridg[i+1-gqx],pgridb[i+1-gqx]);
			M_glVertex3f(*(pgx+1-gqx),*(pgy+1-gqx),*(pgz+1-gqx));

			M_glColor3ub(pgridr[i-gqx],pgridg[i-gqx],pgridb[i-gqx]);
			M_glVertex3f(*(pgx-gqx),*(pgy-gqx),*(pgz-gqx));


			M_glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
			M_glVertex3f(*pgx,*pgy,*pgz);

			M_glColor3ub(pgridr[i-gqx],pgridg[i-gqx],pgridb[i-gqx]);
			M_glVertex3f(*(pgx-gqx),*(pgy-gqx),*(pgz-gqx));

			M_glColor3ub(pgridr[i-gqx-1],pgridg[i-gqx-1],pgridb[i-gqx-1]);
			M_glVertex3f(*(pgx-1-gqx),*(pgy-1-gqx),*(pgz-1-gqx));

			M_glColor3ub(pgridr[i-1],pgridg[i-1],pgridb[i-1]);
			M_glVertex3f(*(pgx-1),*(pgy-1),*(pgz-1));

			count += 4;

			i+=2;
			pgx+=2; pgy+=2; pgz+=2;
		}
		i+=(gqx+2);
		pgx+=(gqx+2); pgy+=(gqx+2); pgz+=(gqx+2);
	}

	renderVertexArrays(4 * count, true, false, false);
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

	float speed=globalTime/512.0f;
	int count = 0;

	glBindTexture(GL_TEXTURE_2D, texture[2]);

 	initGlArrayPointers();

	for (z=z0; z<z1; z+=dz) {
		const unsigned char cz = (unsigned char)((1-z/zsize) * 255);
		for (x=x0; x<x1; x+=dx) {
			M_glColor3ub(cz, cz, cz);
			M_glTexCoord2f(x/(xsize*f),z/(zsize*f)+speed);
			M_glVertex3f(x,y,-z);

			M_glColor3ub(cz, cz, cz);
			M_glTexCoord2f((x+dx)/(xsize*f),z/(zsize*f)+speed);
			M_glVertex3f(x+dx,y,-z);

			M_glColor3ub(cz, cz, cz);
			M_glTexCoord2f((x+dx)/(xsize*f),(z+dz)/(zsize*f)+speed);
			M_glVertex3f(x+dx,y,-z-dz);

			M_glColor3ub(cz, cz, cz);
			M_glTexCoord2f(x/(xsize*f),(z+dz)/(zsize*f)+speed);
			M_glVertex3f(x,y,-z-dz);

			++count;
		}
	}

	renderVertexArrays(4 * count, true, true, false);
}


void VS_Stars2d(float distance, float size, int i0, int i1)
{
	int count = 0;

	glBindTexture(GL_TEXTURE_2D, texture[5]);

	initGlArrayPointers();

	for (int i=i0; i<i1; i++)
	{
		starx[i]+=starspeed[i];
		if (starx[i]<-1024.0f) starx[i]=1024.0f;
		
		const float x = starx[i];
		const float y = stary[i];
		const float z = starz[i] + distance;

		const unsigned char r = starcolr[i];
		const unsigned char g = starcolg[i];
		const unsigned char b = starcolb[i];

		M_glColor3ub(r,g,b);
		M_glTexCoord2f(0,0);
		M_glVertex3f(-size+x,-size+y,z);

		M_glColor3ub(r,g,b);
		M_glTexCoord2f(1,0);
		M_glVertex3f(size+x,-size+y,z);

		M_glColor3ub(r,g,b);
		M_glTexCoord2f(1,1);
		M_glVertex3f(size+x,size+y,z);

		M_glColor3ub(r,g,b);
		M_glTexCoord2f(0,1);
		M_glVertex3f(-size+x,size+y,z);

		++count;
	}

	renderVertexArrays(4 * count, true, true, false);
}


// ======== Distort ========

void VS_Distort()
{
	int i,x,y;
	int count = 0;

	float tpx=0, tpy=0;
	float dpx=1.0/nqx, dpy=1.0/nqy;

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	initGlArrayPointers();

	i=0;
	for (y=0; y<nqy-1; y++)
	{
		tpx=0;

		for (x=0; x<nqx-1; x++)
		{
			M_glTexCoord2f(tpx,tpy);
			M_glVertex3f(gridx[i],gridy[i],gridz[i]);
			M_glColor3ub(gridr[i+nqx],gridg[i+nqx],gridb[i+nqx]);


			M_glTexCoord2f(tpx+dpx,tpy);
			M_glVertex3f(gridx[i+1],gridy[i+1],gridz[i+1]);
			M_glColor3ub(gridr[i],gridg[i],gridb[i]);

			M_glTexCoord2f(tpx+dpx,tpy+dpy);
			M_glVertex3f(gridx[i+1+nqx],gridy[i+1+nqx],gridz[i+1+nqx]);
			M_glColor3ub(gridr[i+1],gridg[i+1],gridb[i+1]);

			M_glTexCoord2f(tpx,tpy+dpy);
			M_glVertex3f(gridx[i+nqx],gridy[i+nqx],gridz[i+nqx]);
			M_glColor3ub(gridr[i+1+nqx],gridg[i+1+nqx],gridb[i+1+nqx]);

			++count;

			i++;
			tpx+=dpx;
		}
		i++;
		tpy+=dpy;
	}

	renderVertexArrays(4 * count, true, true, false);
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
	int i;

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
			for (int i=0; i<nlns; i++)
			{
				glVertex3f(xo[lp0[i]],yo[lp0[i]],zo[lp0[i]]);
				glVertex3f(xo[lp1[i]],yo[lp1[i]],zo[lp1[i]]);
			}
			glEnd();
			*/
		break;


		case 2:
			initGlArrayPointers();

			for (i=0; i<npls; i++)
			{
				M_glNormal3f(pnv[pp0[i]].x,pnv[pp0[i]].y,pnv[pp0[i]].z);
				M_glVertex3f(xo[pp0[i]],yo[pp0[i]],zo[pp0[i]]);

				M_glNormal3f(pnv[pp1[i]].x,pnv[pp1[i]].y,pnv[pp1[i]].z);
				M_glVertex3f(xo[pp1[i]],yo[pp1[i]],zo[pp1[i]]);

				M_glNormal3f(pnv[pp2[i]].x,pnv[pp2[i]].y,pnv[pp2[i]].z);
				M_glVertex3f(xo[pp2[i]],yo[pp2[i]],zo[pp2[i]]);
			}

			renderVertexArrays(3 * npls, false, false, true, true);
		break;

		default:
			break;
	}

//	RenderLight();
}


/*void RenderLight()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	VS_Blob_Begin();
	VS_Blob(LightPosition[0], LightPosition[1], LightPosition[2],255,255,255,32.0f);
	VS_Blob_End();
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}*/


void VS_Stars3d()
{
	initGlArrayPointers();

	glBindTexture(GL_TEXTURE_2D, texture[5]);

	const float bsize = 1.5f;

	for (int i=0; i<1024; i++)
	{
		const float mulc=(256.0f+star[i].z)/256.0f;
		const float x = star[i].x;
		const float y = star[i].y;
		const float z = star[i].z;
		const unsigned char r = (unsigned char)(star[i].rcol*mulc);
		const unsigned char g = (unsigned char)(star[i].gcol*mulc);
		const unsigned char b = (unsigned char)(star[i].bcol*mulc);

		M_glColor3ub(r,g,b);
		M_glVertex3f(-bsize+x,-bsize+y,z);

		M_glColor3ub(r,g,b);
		M_glVertex3f(bsize+x,-bsize+y,z);

		M_glColor3ub(r,g,b);
		M_glVertex3f(bsize+x,bsize+y,z);

		M_glColor3ub(r,g,b);
		M_glVertex3f(-bsize+x,bsize+y,z);
	}

	renderVertexArrays(4 * 1024, true, true, false);
}

void VS_Prepare_Blob_TC(int count)
{
	glArrayTexcoordsPtr = glArrayTexcoords;

	for (int th = 0; th < count; ++th) {
		*glArrayTexcoordsPtr++ = 0; *glArrayTexcoordsPtr++ = 0;
		*glArrayTexcoordsPtr++ = 1; *glArrayTexcoordsPtr++ = 0;
		*glArrayTexcoordsPtr++ = 1; *glArrayTexcoordsPtr++ = 1;
		*glArrayTexcoordsPtr++ = 0; *glArrayTexcoordsPtr++ = 1;
	}
}

void VS_Flower(flower flo)
{
	const float thk=globalTime / 128.0f;
	float const mulc = (256.0f + flo.zfp) / 256.0f;
	const float bsize = 1.35f;

	initGlArrayPointers();

	glColor3ub(	(unsigned char)((255.0f/flo.rdiv)*mulc), 
				(unsigned char)((255.0f/flo.gdiv)*mulc), 
				(unsigned char)((255.0f/flo.bdiv)*mulc));

	for (int th = 0; th < VS_FLOWER_POINTS; ++th)
	{
		const float theta = (float)th;
		const float ro = sin((theta + flo.tmul1 * thk)/(D2R/flo.angdiv1)) * flo.sinmul1 + cos((theta + flo.tmul2 * thk)/(D2R/flo.angdiv2)) * flo.sinmul2 + flo.fsize;
		const float fx=ro*cos(theta/D2R) + flo.xfp;
		const float fy=ro*sin(theta/D2R) + flo.yfp;
		const float fz=flo.zfp;

		M_glVertex3f(-bsize+fx,-bsize+fy, fz);
		M_glVertex3f(bsize+fx,-bsize+fy, fz);
		M_glVertex3f(bsize+fx,bsize+fy, fz);
		M_glVertex3f(-bsize+fx,bsize+fy, fz);
	}

	renderVertexArrays(4*VS_FLOWER_POINTS, false, true, false);
}

void VS_Blob_Begin()
{
	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glBegin(GL_QUADS);
}

void VS_Blob_End()
{
	glEnd();
}


void VS_Blob(float x, float y, float z, unsigned char r, unsigned char g, unsigned char b, float bsize)
{
	glColor3ub(r,g,b);

	glTexCoord2f(0,0);
	glVertex3f(-bsize+x,-bsize+y,z);

	glTexCoord2f(1,0);
	glVertex3f(bsize+x,-bsize+y,z);

	glTexCoord2f(1,1);
	glVertex3f(bsize+x,bsize+y,z);

	glTexCoord2f(0,1);
	glVertex3f(-bsize+x,bsize+y,z);
}

// We must do everything with element indices then it would be quite faster
void VS_Water(int texn, float px, float py)
{
	float tpx, tpy;
	float dpx=2.0f/wqx, dpy=2.0f/wqy;

	glBindTexture(GL_TEXTURE_2D, texture[texn]);


	initGlArrayPointers();
	int ii = 0;

	int i=wqx+1;
	tpy=0.0f+py;

	float yq = (float)(1-(wqy>>1));
	for (int y=1; y<wqy; y++)
	{
		tpx=0.0f+px;
		float xq = (float)(1-(wqx>>1));
		for (int x=1; x<wqx; x++)
		{
			N_glTexCoord2f(tpx,tpy);
			N_glNormal3f(nbufferx[i],nbuffery[i],nbufferz[i]);
			N_glVertex3f(xq,yq,hbuffer[i]);

			/*N_glTexCoord2f(tpx+dpx,tpy);
			N_glNormal3f(nbufferx[i+1],nbuffery[i+1],nbufferz[i+1]);
			N_glVertex3f(xq+1,yq,hbuffer[i+1]);

			N_glTexCoord2f(tpx+dpx,tpy+dpy);
			N_glNormal3f(nbufferx[i+1+wqx],nbuffery[i+1+wqx],nbufferz[i+1+wqx]);
			N_glVertex3f(xq+1,yq+1,hbuffer[i+1+wqx]);

			N_glTexCoord2f(tpx,tpy+dpy);
			N_glNormal3f(nbufferx[i+wqx],nbuffery[i+wqx],nbufferz[i+wqx]);
			N_glVertex3f(xq,yq+1,hbuffer[i+wqx]);*/

			//Quads
			if ((x<wqx-1) && (y<wqy-1)) {
				N_glIndex4us(ii,ii+1,ii+1+wqx-1,ii+wqx-1);
			}

			//QuadStrips
			//N_glIndex2us(ii,ii+wqx-1);
			++ii;

			tpx+=dpx;
			i++;
			xq++;
		}
		yq++;
		i+=1;
		tpy+=dpy;
	}

	//QuadStrips
	//renderVertexArraysPacked(2 * (wqx-1) * (wqy-2), true, false, true, true, true);

	//Quads
	renderVertexArraysPacked(4 * (wqx-2) * (wqy-2), true, false, true, true, false);
}

// Why this one faster from above? Or above at least not improved?
void VS_Water0(int texn, float px, float py)
{
	int xq,yq;

	float tpx, tpy;
	float dpx=2.0f/wqx, dpy=2.0f/wqy;



	glBindTexture(GL_TEXTURE_2D, texture[texn]);

	glBegin(GL_QUADS);

			int i=wqx+1;
			tpy=0.0f+py;
			for (int y=1; y<wqy-1; y++)
			{
				tpx=0.0f+px;
				yq=y-(wqy>>1);
				for (int x=1; x<wqx-1; x++)
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
	glBindTexture(GL_TEXTURE_2D, texture[6]);

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

	glBindTexture(GL_TEXTURE_2D, texture[8]);
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


	float wtrx=globalTime/12288.0f;
	float wtry=globalTime/-18432.0f;

	glEnable(GL_TEXTURE_2D);
	VS_WannabeCaustics(2, wtrx, wtry, height2);


	glBindTexture(GL_TEXTURE_2D, texture[7]);

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
	int x,y;
	int xq,yq;
	int jumps=4;
	int shit=wqx/jumps;
	float ficken=4.0f;

	float tpx, tpy;
	float dpx=(float)jumps*ficken/wqx, dpy=float(jumps)*ficken/wqy;

	float dispx[wqx*wqy];
	float dispy[wqx*wqy];

	int count = 0;

	int i=wqx+1;
	tpy=0.0f+py;
	for (y=0; y<wqy; y+=jumps)
	{
		tpx=0.0f+px;
		yq=y-(wqy>>1);
		for (x=0; x<wqx; x+=jumps)
		{
			xq=x-(wqx>>1);

			dispx[i]=tpx+sin((y+globalTime/128.0f)/4.0f)*0.03f;
			dispy[i]=tpy+sin((x+globalTime/192.0f)/4.0f)*0.05f;

			tpx+=dpx;
			i++;
		}
		tpy+=dpy;
	}


	glBindTexture(GL_TEXTURE_2D, texture[texn]);

	initGlArrayPointers();

	i=wqx+1;
	tpy=0.0f+py;
	for (y=1; y<wqy-1; y+=jumps)
	{
		yq=y-(wqy>>1);
		for (x=1; x<wqx-1; x+=jumps)
		{
			xq=x-(wqx>>1);

			M_glTexCoord2f(dispx[i],dispy[i]);
			M_glVertex3f(xq,yq,hgt);

			M_glTexCoord2f(dispx[i+1],dispy[i+1]);
			M_glVertex3f(xq+jumps,yq,hgt);

			M_glTexCoord2f(dispx[i+shit+1],dispy[i+shit+1]);
			M_glVertex3f(xq+jumps,yq+jumps,hgt);

			M_glTexCoord2f(dispx[i+shit],dispy[i+shit]);
			M_glVertex3f(xq,yq+jumps,hgt);

			++count;

			i++;
		}
	}

	renderVertexArrays(4 * count, false, true, false);
}


void VS_Spherical()
{
	//int xq,yq;
	int x,y;

	float tpx, tpy;
	float dpx=1.0f/wqx, dpy=1.0f/wqy;


	glBindTexture(GL_TEXTURE_2D, texture[3]);

	initGlArrayPointers();

	tpy=0.0f;
	int i=spx+1;
	int count = 0;
	for (y=1; y<spy-1; y+=2)
	{

		tpx=0.0f;
		//yq=y-(wqy>>1);
		for (x=1; x<spx-1; x+=2)
		{
			//xq=x-(wqx>>1);

			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[i],sphg[i],sphb[i]);
			M_glVertex3f(sphx[i],sphy[i],sphz[i]);

			M_glTexCoord2f(tpx-dpx,tpy);
			M_glColor3ub(sphr[i-1],sphg[i-1],sphb[i-1]);
			M_glVertex3f(sphx[i-1],sphy[i-1],sphz[i-1]);

			M_glTexCoord2f(tpx-dpx,tpy+dpy);
			M_glColor3ub(sphr[i-1+spx],sphg[i-1+spx],sphb[i-1+spx]);
			M_glVertex3f(sphx[i-1+spx],sphy[i-1+spx],sphz[i-1+spx]);

			M_glTexCoord2f(tpx,tpy+dpy);
			M_glColor3ub(sphr[i+spx],sphg[i+spx],sphb[i+spx]);
			M_glVertex3f(sphx[i+spx],sphy[i+spx],sphz[i+spx]);


			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[i],sphg[i],sphb[i]);
			M_glVertex3f(sphx[i],sphy[i],sphz[i]);

			M_glTexCoord2f(tpx,tpy+dpy);
			M_glColor3ub(sphr[i+spx],sphg[i+spx],sphb[i+spx]);
			M_glVertex3f(sphx[i+spx],sphy[i+spx],sphz[i+spx]);

			M_glTexCoord2f(tpx+dpx,tpy+dpy);
			M_glColor3ub(sphr[i+spx+1],sphg[i+spx+1],sphb[i+spx+1]);
			M_glVertex3f(sphx[i+spx+1],sphy[i+spx+1],sphz[i+spx+1]);

			M_glTexCoord2f(tpx+dpx,tpy);
			M_glColor3ub(sphr[i+1],sphg[i+1],sphb[i+1]);
			M_glVertex3f(sphx[i+1],sphy[i+1],sphz[i+1]);


			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[i],sphg[i],sphb[i]);
			M_glVertex3f(sphx[i],sphy[i],sphz[i]);

			M_glTexCoord2f(tpx+dpx,tpy);
			M_glColor3ub(sphr[i+1],sphg[i+1],sphb[i+1]);
			M_glVertex3f(sphx[i+1],sphy[i+1],sphz[i+1]);

			M_glTexCoord2f(tpx+dpx,tpy-dpy);
			M_glColor3ub(sphr[i+1-spx],sphg[i+1-spx],sphb[i+1-spx]);
			M_glVertex3f(sphx[i+1-spx],sphy[i+1-spx],sphz[i+1-spx]);

			M_glTexCoord2f(tpx,tpy-dpy);
			M_glColor3ub(sphr[i-spx],sphg[i-spx],sphb[i-spx]);
			M_glVertex3f(sphx[i-spx],sphy[i-spx],sphz[i-spx]);


			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[i],sphg[i],sphb[i]);
			M_glVertex3f(sphx[i],sphy[i],sphz[i]);

			M_glTexCoord2f(tpx,tpy-dpy);
			M_glColor3ub(sphr[i-spx],sphg[i-spx],sphb[i-spx]);
			M_glVertex3f(sphx[i-spx],sphy[i-spx],sphz[i-spx]);

			M_glTexCoord2f(tpx-dpx,tpy-dpy);
			M_glColor3ub(sphr[i-spx-1],sphg[i-spx-1],sphb[i-spx-1]);
			M_glVertex3f(sphx[i-spx-1],sphy[i-spx-1],sphz[i-spx-1]);

			M_glTexCoord2f(tpx-dpx,tpy);
			M_glColor3ub(sphr[i-1],sphg[i-1],sphb[i-1]);
			M_glVertex3f(sphx[i-1],sphy[i-1],sphz[i-1]);

			count += 4;
			tpx+=(dpx*2);
			i+=2;
		}
		tpy+=(dpy*2);
		i+=spx+2;
		
	}

// correction

	tpx=0.0f;
		int j=(spy-1)*spx+1; i=1;
		for (x=1; x<spx-1; x+=2)
		{

			M_glTexCoord2f(tpx,tpy-dpy);
			M_glColor3ub(sphr[j-spx],sphg[j-spx],sphb[j-spx]);
			M_glVertex3f(sphx[j-spx],sphy[j-spx],sphz[j-spx]);

			M_glTexCoord2f(tpx+dpx,tpy-dpy);
			M_glColor3ub(sphr[j-spx+1],sphg[j-spx+1],sphb[j-spx+1]);
			M_glVertex3f(sphx[j-spx+1],sphy[j-spx+1],sphz[j-spx+1]);

			M_glTexCoord2f(tpx+dpx,tpy);
			M_glColor3ub(sphr[j+1],sphg[j+1],sphb[j+1]);
			M_glVertex3f(sphx[j+1],sphy[j+1],sphz[j+1]);

			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[j],sphg[j],sphb[j]);
			M_glVertex3f(sphx[j],sphy[j],sphz[j]);


			M_glTexCoord2f(tpx,tpy-dpy);
			M_glColor3ub(sphr[j-spx],sphg[j-spx],sphb[j-spx]);
			M_glVertex3f(sphx[j-spx],sphy[j-spx],sphz[j-spx]);

			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[j],sphg[j],sphb[j]);
			M_glVertex3f(sphx[j],sphy[j],sphz[j]);

			M_glTexCoord2f(tpx-dpx,tpy);
			M_glColor3ub(sphr[j-1],sphg[j-1],sphb[j-1]);
			M_glVertex3f(sphx[j-1],sphy[j-1],sphz[j-1]);

			M_glTexCoord2f(tpx-dpx,tpy-dpy);
			M_glColor3ub(sphr[j-spx-1],sphg[j-spx-1],sphb[j-spx-1]);
			M_glVertex3f(sphx[j-spx-1],sphy[j-spx-1],sphz[j-spx-1]);


			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[j],sphg[j],sphb[j]);
			M_glVertex3f(sphx[j],sphy[j],sphz[j]);

			M_glTexCoord2f(tpx+dpx,tpy);
			M_glColor3ub(sphr[j+1],sphg[j+1],sphb[j+1]);
			M_glVertex3f(sphx[j+1],sphy[j+1],sphz[j+1]);

			M_glTexCoord2f(tpx+dpx,tpy+dpy);
			M_glColor3ub(sphr[i+1],sphg[i+1],sphb[i+1]);
			M_glVertex3f(sphx[i+1],sphy[i+1],sphz[i+1]);

			M_glTexCoord2f(tpx,tpy+dpy);
			M_glColor3ub(sphr[i],sphg[i],sphb[i]);
			M_glVertex3f(sphx[i],sphy[i],sphz[i]);


			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[j],sphg[j],sphb[j]);
			M_glVertex3f(sphx[j],sphy[j],sphz[j]);

			M_glTexCoord2f(tpx,tpy+dpy);
			M_glColor3ub(sphr[i],sphg[i],sphb[i]);
			M_glVertex3f(sphx[i],sphy[i],sphz[i]);

			M_glTexCoord2f(tpx-dpx,tpy+dpy);
			M_glColor3ub(sphr[i-1],sphg[i-1],sphb[i-1]);
			M_glVertex3f(sphx[i-1],sphy[i-1],sphz[i-1]);

			M_glTexCoord2f(tpx-dpx,tpy);
			M_glColor3ub(sphr[j-1],sphg[j-1],sphb[j-1]);
			M_glVertex3f(sphx[j-1],sphy[j-1],sphz[j-1]);

			count += 4;
			tpx+=(dpx*2);
			i+=2;
			j+=2;
		}

		renderVertexArrays(4 * count, true, true, false);
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
		tail0[i]=sin((i+globalTime/s0.speed)/s0.sinediv) * s0.sinemul + s0.sineadd;
		tail1[i]=sin((i+globalTime/s1.speed)/s1.sinediv) * s1.sinemul + s1.sineadd;
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

	Vector view;
	view.x = 0.0f;
	view.y = 0.0f;
	view.z = -1.0f;

	Vector side1, side2, sidenormal;

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
	float cosxr=cos(rpx/D2R); float cosyr=cos(rpy/D2R); float coszr=cos(rpz/D2R);
	float sinxr=sin(rpx/D2R); float sinyr=sin(rpy/D2R); float sinzr=sin(rpz/D2R);

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

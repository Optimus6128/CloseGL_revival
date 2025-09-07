#pragma warning(disable: 4244)
#pragma warning(disable: 4809)
#pragma warning(disable: 4305)

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "glext.h"
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

extern bool pointSpritesSupported;


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

#define M_glIndex4us(p0,p1,p2,p3) *glArrayIndexPtr++ = (p0); *glArrayIndexPtr++ = (p1); *glArrayIndexPtr++ = (p2); *glArrayIndexPtr++ = (p3);
#define M_glIndex2us(p0,p1) *glArrayIndexPtr++ = (p0); *glArrayIndexPtr++ = (p1);

#define MP_glColor3ub(cr,cg,cb) glArrayVertexDataPtr->color.r = (cr); glArrayVertexDataPtr->color.g = (cg); glArrayVertexDataPtr->color.b = (cb);
#define MP_glTexCoord2f(tu,tv) glArrayVertexDataPtr->texcoord.u = (tu); glArrayVertexDataPtr->texcoord.v = (tv);
#define MP_glNormal3f(px,py,pz) glArrayVertexDataPtr->normal.x = (px); glArrayVertexDataPtr->normal.y = (py); glArrayVertexDataPtr->normal.z = (pz);
#define MP_glVertex3f(px,py,pz) glArrayVertexDataPtr->position.x = (px); glArrayVertexDataPtr->position.y = (py); glArrayVertexDataPtr->position.z = (pz); ++glArrayVertexDataPtr;

// Unpacked separate arrays are slightly faster here? Let's test..

static Vector glArrayVertices[MAX_ARRAY_SIZE];
static Color glArrayColors3ub[MAX_ARRAY_SIZE];
static TexCoords glArrayTexcoords[MAX_ARRAY_SIZE];
static Vector glArrayNormals[MAX_ARRAY_SIZE];

static Vector *glArrayVerticesPtr;
static Color *glArrayColors3ubPtr;
static TexCoords *glArrayTexcoordsPtr;
static Vector *glArrayNormalsPtr;

#define M_glVertex3f(px,py,pz) glArrayVerticesPtr->x = (px); glArrayVerticesPtr->y = (py); glArrayVerticesPtr->z = (pz); ++glArrayVerticesPtr;
#define M_glColor3ub(cr,cg,cb) glArrayColors3ubPtr->r = (cr); glArrayColors3ubPtr->g = (cg); glArrayColors3ubPtr->b = (cb); ++glArrayColors3ubPtr;
#define M_glTexCoord2f(tu,tv) glArrayTexcoordsPtr->u = (tu); glArrayTexcoordsPtr->v = (tv); ++glArrayTexcoordsPtr;
#define M_glNormal3f(px,py,pz) glArrayNormalsPtr->x = (px); glArrayNormalsPtr->y = (py); glArrayNormalsPtr->z = (pz); ++glArrayNormalsPtr;

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

static void renderVertexArrays(int count, GLenum type, bool hasIndices, bool hasColors, bool hasTexcoords, bool hasNormals, int offset = 0)
{
	// Setup
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &glArrayVertices[offset]);

	if (hasColors) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(Color), &glArrayColors3ub[offset]);
	}

	if (hasNormals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &glArrayNormals[offset]);
	}

	if (hasTexcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, &glArrayTexcoords[offset]);
	}

	// Draw
	if (hasIndices) {
		glDrawElements(type, count, GL_UNSIGNED_SHORT, glArrayIndices);
	} else {
		glDrawArrays(type, 0, count);
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

static void renderVertexArraysPacked(int count, GLenum type, bool hasIndices, bool hasColors, bool hasTexcoords, bool hasNormals, int offset = 0)
{
	// Setup
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(VertexData), &glArrayVertexData[offset].position);

	if (hasColors) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(VertexData), &glArrayVertexData[offset].color);
	}
	if (hasNormals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, sizeof(VertexData), &glArrayVertexData[offset].normal);
	}
	if (hasTexcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData), &glArrayVertexData[offset].texcoord);
	}

	// Draw
	if (hasIndices) {
		glDrawElements(type, count, GL_UNSIGNED_SHORT, glArrayIndices);
	} else {
		glDrawArrays(type, 0, count);
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

	int i=pqx+1;
	for (int y=1; y<pqy-1; y++)
	{
		int count = 0;
		initGlArrayPointers();

		yq=y-(pqy>>1);
		for (int x=1; x<pqx-1; x++)
		{
			xq=x-(pqx>>1);

			M_glColor3ub(plgridr[i],plgridg[i],plgridb[i]);
			M_glVertex3f(xq,yq,plgrida[i]);

			M_glColor3ub(plgridr[i+pqx],plgridg[i+pqx],plgridb[i+pqx]);
			M_glVertex3f(xq,yq+1,plgrida[i+pqx]);

			count++;

			i++;
		}
		renderVertexArrays(2 * count, GL_QUAD_STRIP, false, true, false, false);
		i+=2;
	}

}



void VS_FlatGridNew(int face)
{
	float *pgx, *pgy, *pgz;
	int count = 0;

	pgx=&gx[face][0];
	pgy=&gy[face][0];
	pgz=&gz[face][0];

	int i=0;//1+gqx;
	for (int y=0; y<gqy-1; y+=1)
	{
		int count = 0;
		initGlArrayPointers();

		for (int x=0; x<gqx; x+=1)
		{
			M_glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
			M_glVertex3f(*pgx,*pgy,*pgz);

			M_glColor3ub(pgridr[i+gqx],pgridg[i+gqx],pgridb[i+gqx]);
			M_glVertex3f(*(pgx+gqx),*(pgy+gqx),*(pgz+gqx));

			/*M_glColor3ub(pgridr[i],pgridg[i],pgridb[i]);
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
			M_glVertex3f(*(pgx-1),*(pgy-1),*(pgz-1));*/

			count += 1;

			i+=1;
			pgx+=1; pgy+=1; pgz+=1;
		}
		//i+=1;
		//pgx+=1; pgy+=1; pgz+=1;

		renderVertexArrays(2 * count, GL_QUAD_STRIP, false, true, false, false);

		//i+=(gqx+2);
		//pgx+=(gqx+2); pgy+=(gqx+2); pgz+=(gqx+2);
	}
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

	renderVertexArrays(4 * count, GL_QUADS, false, true, true, false);
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

	renderVertexArrays(4 * count, GL_QUADS, false, true, true, false);
}


// ======== Distort ========

void VS_Distort()
{
	int i,x,y;

	float tpx=0, tpy=0;
	float dpx=1.0/nqx, dpy=1.0/nqy;

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// After converting to GL_QUAD_STRIP for speed, I cannot recreate the original pixelgrid artifact
	// In the original I was passing glColor after glVertex so I moved colors one off. With quad strips of course I can't recreate it or I just got tired. Doesn't matter, it's close to the original. Worth the speed.
	i=0;
	for (y=0; y<nqy-1; y++)
	{
		int count = 0;
		initGlArrayPointers();

		tpx=0;
		for (x=0; x<nqx-1; x++)
		{
			M_glTexCoord2f(tpx,tpy);
			//M_glColor3ub(gridr[i],gridg[i],gridb[i]);
			M_glColor3ub(gridr[i+1],gridg[i+1],gridb[i+1]);
			//M_glColor3ub(gridr[i+1+nqx],gridg[i+1+nqx],gridb[i+1+nqx]);
			//M_glColor3ub(gridr[i+nqx],gridg[i+nqx],gridb[i+nqx]);
			M_glVertex3f(gridx[i],gridy[i],gridz[i]);

			//M_glTexCoord2f(tpx+dpx,tpy);
			//M_glColor3ub(gridr[i],gridg[i],gridb[i]);
			//M_glVertex3f(gridx[i+1],gridy[i+1],gridz[i+1]);

			//M_glTexCoord2f(tpx+dpx,tpy+dpy);
			//M_glColor3ub(gridr[i+1],gridg[i+1],gridb[i+1]);
			//M_glVertex3f(gridx[i+1+nqx],gridy[i+1+nqx],gridz[i+1+nqx]);

			M_glTexCoord2f(tpx,tpy+dpy);
			//M_glColor3ub(gridr[i],gridg[i],gridb[i]);
			//M_glColor3ub(gridr[i+1],gridg[i+1],gridb[i+1]);
			//M_glColor3ub(gridr[i+1+nqx],gridg[i+1+nqx],gridb[i+1+nqx]);
			M_glColor3ub(gridr[i+nqx],gridg[i+nqx],gridb[i+nqx]);
			M_glVertex3f(gridx[i+nqx],gridy[i+nqx],gridz[i+nqx]);

			++count;

			i++;
			tpx+=dpx;
		}
		renderVertexArrays(2 * count, GL_QUAD_STRIP, false, true, true, false);
		i++;
		tpy+=dpy;
	}
}

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

			renderVertexArrays(3 * npls, GL_TRIANGLES, false, false, false, true);
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

	renderVertexArrays(4 * 1024, GL_QUADS, false, true, true, false);
}

void VS_Prepare_Blob_TC(int count)
{
	initGlArrayPointers();

	for (int th = 0; th < count; ++th) {
		M_glTexCoord2f(0,0);// ++glArrayVertexDataPtr;
		M_glTexCoord2f(1,0);// ++glArrayVertexDataPtr;
		M_glTexCoord2f(1,1);// ++glArrayVertexDataPtr;
		M_glTexCoord2f(0,1);// ++glArrayVertexDataPtr;
	}
}

static float sinThetaTab[VS_FLOWER_POINTS];
static float cosThetaTab[VS_FLOWER_POINTS];

static void initVS_Flower()
{
	for (int i = 0; i < VS_FLOWER_POINTS; ++i)
	{
		sinThetaTab[i] = sinf((float)i/D2R);
		cosThetaTab[i] = cosf((float)i/D2R);
	}
}


void VS_Flower(flower flo)
{
	static bool isVS_FlowerInit = false;
	if (!isVS_FlowerInit) {
		initVS_Flower();
		isVS_FlowerInit = true;
	}

	const float thk=globalTime / 128.0f;
	float const mulc = (256.0f + flo.zfp) / 256.0f;
	const float bsize = 1.35f;

	initGlArrayPointers();

	glColor3ub(	(unsigned char)((255.0f/flo.rdiv)*mulc), 
				(unsigned char)((255.0f/flo.gdiv)*mulc), 
				(unsigned char)((255.0f/flo.bdiv)*mulc));

	for (int i = 0; i < VS_FLOWER_POINTS; ++i)
	{
		const float theta = (float)i;
		const float ro = sinf((theta + flo.tmul1 * thk)/(D2R/flo.angdiv1)) * flo.sinmul1 + cosf((theta + flo.tmul2 * thk)/(D2R/flo.angdiv2)) * flo.sinmul2 + flo.fsize;
		const float fx=ro*cosThetaTab[i] + flo.xfp;
		const float fy=ro*sinThetaTab[i] + flo.yfp;
		const float fz=flo.zfp;

		/*if (pointSpritesSupported) {
			M_glVertex3f(fx,fy,fz);
		} else {*/
			M_glVertex3f(-bsize+fx,-bsize+fy, fz);
			M_glVertex3f(bsize+fx,-bsize+fy, fz);
			M_glVertex3f(bsize+fx,bsize+fy, fz);
			M_glVertex3f(-bsize+fx,bsize+fy, fz);
		//}
	}

	/*if (pointSpritesSupported) {
		renderVertexArrays(VS_FLOWER_POINTS, GL_POINTS, false, false, false, false);
	} else {*/
		renderVertexArrays(4*VS_FLOWER_POINTS, GL_QUADS, false, false, true, false);
	//}
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
	int x,y;
	float tpx, tpy;
	float dpx=2.0f/wqx, dpy=2.0f/wqy;

	glBindTexture(GL_TEXTURE_2D, texture[texn]);

	initGlArrayPointers();
	int ii = 0;

	int i=wqx+1;
	tpy=0.0f+py;

	float yq = (float)(1-(wqy>>1));
	for (y=1; y<wqy; y++)
	{
		tpx=0.0f+px;
		float xq = (float)(1-(wqx>>1));
		for (x=1; x<wqx; x++)
		{
			M_glTexCoord2f(tpx,tpy);
			M_glNormal3f(nbufferx[i],nbuffery[i],nbufferz[i]);
			M_glVertex3f(xq,yq,hbuffer[i]);

			/*M_glTexCoord2f(tpx+dpx,tpy);
			M_glNormal3f(nbufferx[i+1],nbuffery[i+1],nbufferz[i+1]);
			M_glVertex3f(xq+1,yq,hbuffer[i+1]);

			M_glTexCoord2f(tpx+dpx,tpy+dpy);
			M_glNormal3f(nbufferx[i+1+wqx],nbuffery[i+1+wqx],nbufferz[i+1+wqx]);
			M_glVertex3f(xq+1,yq+1,hbuffer[i+1+wqx]);

			M_glTexCoord2f(tpx,tpy+dpy);
			M_glNormal3f(nbufferx[i+wqx],nbuffery[i+wqx],nbufferz[i+wqx]);
			M_glVertex3f(xq,yq+1,hbuffer[i+wqx]);*/

			//QuadStrips
			M_glIndex2us(ii,ii+wqx-1);
			++ii;

			tpx+=dpx;
			i++;
			xq++;
		}

		yq++;
		i+=1;
		tpy+=dpy;
	}

	for (y=1; y<wqy-1; y++)
	{
		const int indicesPerRow = 2 * (wqx-1);
		renderVertexArrays(indicesPerRow, GL_QUAD_STRIP, true, false, true, true, (y-1) * (indicesPerRow /2));
	}
}

void VS_PisinaGrid(float xsize, float ysize, float xpos, float ypos, float tscale, float height)
{

	float plght=0.25f;
	float pdrkn=0.0f;

	glTexCoord2f(0.0f,tscale);
	glVertex3f(0.0f + xpos, 0.0f + ypos, height);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(xsize + xpos, 0.0f + ypos, height);
	glTexCoord2f(tscale,0.0f);
	glVertex3f(xsize + xpos, ysize + ypos, height);
	glTexCoord2f(tscale,tscale);
	glVertex3f(0.0f + xpos, ysize + ypos, height);
}


void VS_Pisina(float height1, float height2)
{
	float psqx=(wqx-2)/2.1f;
	float psqy=(wqy-2)/2.1f;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[6]);

	glDisable(GL_CULL_FACE);

	// Right side
	glColor3f(1.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(psqx, -psqy, height1);
		glTexCoord2f(4.0f,1.0f);
		glVertex3f(psqx, -psqy, height2);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(psqx, psqy, height2);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx, psqy, height1);

	// Front side
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx, psqy, height1);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(psqx, psqy, height2);
		glTexCoord2f(4.0f,1.0f);
		glVertex3f(-psqx, psqy, height2);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(-psqx, psqy, height1);

	// Left side
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

		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(psqx*bxscale, psqy*bxscale, height3);
		glTexCoord2f(0.0f,2.0f);
		glVertex3f(psqx*bxscale, psqy*bxscale, height4);
		glTexCoord2f(4.0f,2.0f);
		glVertex3f(-psqx*bxscale, psqy*bxscale, height4);
		glTexCoord2f(4.0f,0.0f);
		glVertex3f(-psqx*bxscale, psqy*bxscale, height3);

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

	float wtrx=globalTime/12288.0f;
	float wtry=globalTime/-18432.0f;

	glEnable(GL_TEXTURE_2D);
	VS_WannabeCaustics(2, wtrx, wtry, height2);

	glBindTexture(GL_TEXTURE_2D, texture[7]);
	glNormal3f(0.0f,0.0f,0.5f);

	glBegin(GL_QUADS);
		VS_PisinaGrid(psqx*2, psqy*2, psqx, -psqy, 4.0f, height1);
		VS_PisinaGrid(psqx*2, psqy*2, psqx, psqy, 4.0f, height1);
		VS_PisinaGrid(psqx*2, psqy*2, psqx, -3*psqy, 4.0f, height1);

		VS_PisinaGrid(psqx*2, psqy*2, -3*psqx, -psqy, 4.0f, height1);
		VS_PisinaGrid(psqx*2, psqy*2, -3*psqx, psqy, 4.0f, height1);
		VS_PisinaGrid(psqx*2, psqy*2, -3*psqx, -3*psqy, 4.0f, height1);

		VS_PisinaGrid(psqx*2, psqy*2, -psqx, psqy, 4.0f, height1);
		VS_PisinaGrid(psqx*2, psqy*2, -psqx, -3*psqy, 4.0f, height1);
	glEnd();
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

			float dspx = tpx+sinf((y+globalTime/128.0f)/4.0f)*0.03f;
			float dspy = tpy+sinf((x+globalTime/192.0f)/4.0f)*0.05f;

			dispx[i]= dspx;
			dispy[i]= dspy;

			tpx+=dpx;
			i++;
		}
		tpy+=dpy;
	}


	glBindTexture(GL_TEXTURE_2D, texture[texn]);

	initGlArrayPointers();

	i=wqx+1;
	tpy=0.0f+py;
	for (y=3; y<wqy-1-jumps; y+=jumps)
	{
		yq=y-(wqy>>1)+1;
		for (x=2; x<wqx-1-jumps; x+=jumps)
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
		i++;
	}

	renderVertexArrays(4 * count, GL_QUADS, false, false, true, false);
}


void VS_Spherical()
{
	int x,y;

	float tpx, tpy;
	float dpx=1.0f/wqx, dpy=1.0f/wqy;


	glBindTexture(GL_TEXTURE_2D, texture[3]);

	tpy=0.0f;
	int i=spx+1;

	for (y=1; y<spy-1; y+=1)
	{
		int count = 0;
		initGlArrayPointers();

		tpx=0.0f;
		for (x=1; x<spx-1; x+=1)
		{
			M_glTexCoord2f(tpx,tpy);
			M_glColor3ub(sphr[i],sphg[i],sphb[i]);
			M_glVertex3f(sphx[i],sphy[i],sphz[i]);

			M_glTexCoord2f(tpx,tpy+dpy);
			M_glColor3ub(sphr[i+spx],sphg[i+spx],sphb[i+spx]);
			M_glVertex3f(sphx[i+spx],sphy[i+spx],sphz[i+spx]);

			count += 1;
			tpx+=dpx;
			i+=1;
		}
		renderVertexArrays(2 * count, GL_QUAD_STRIP, false, true, true, false);

		tpy+=dpy;
		i+=2;
	}
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

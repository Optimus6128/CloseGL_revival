#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opengl.h"
#include "TextureLoad.h"


const int ntex=9;
int ntexs=ntex;
int tex;

unsigned int png, png_alpha, jpeg;


GLuint texture[ntex];
GLuint texPolar;

// ======= GL Texture Loading =======

struct bmphdr {
	/*char magic[2];*/					/* BM */
	unsigned int filesz;
	unsigned int rsvd;
	unsigned int dataoffs;			/* offset to pixels */
	unsigned int hdrsize;
	int width, height;
	unsigned short num_planes, num_bits;
	unsigned int compression;
	unsigned int sizeimg;
	int xppm, yppm;
	unsigned int num_colors, num_important;
};

static void bswap16(unsigned short *val)
{
	unsigned short x = *val;
	*val = (x >> 8) | (x << 8);
}
static void bswap32(unsigned int *val)
{
	unsigned int x = *val;
	*val = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
}

static int convbmp8(Image *img, struct bmphdr *hdr, FILE *fp, const char *fname);
static int convbmp24(Image *img, struct bmphdr *hdr, FILE *fp, const char *fname);

Image *LoadBMP(const char *fname)
{
	char magic[2];
	unsigned int infoffs, imgsize;
	FILE *fp = 0;
	Image *img;
	struct bmphdr hdr;
	unsigned int bpp;

	int bigend = (*(unsigned int*)"ABCD" == 0x41424344);

	if(!(img = (Image*)calloc(1, sizeof *img))) {
		goto err;
	}
	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "failed to load: %s\n", fname);
		goto err;
	}

	if(fgetc(fp) != 'B' || fgetc(fp) != 'M') {
		fprintf(stderr, "invalid bitmap file: %s\n", fname);
		goto err;
	}

	if(fread(&hdr, sizeof hdr, 1, fp) <= 0) {
		fprintf(stderr, "unexpected EOF while reading bitmap header: %s\n", fname);
		goto err;
	}
	if(bigend) {
		bswap32(&hdr.hdrsize);
		bswap32(&hdr.dataoffs);
		bswap32((unsigned int*)&hdr.width);
		bswap32((unsigned int*)&hdr.height);
		bswap16(&hdr.num_bits);
		bswap16(&hdr.num_planes);
		bswap32(&hdr.compression);
	}

	if(hdr.compression) {
		fprintf(stderr, "%s: unsupported compressed bitmap\n", fname);
		goto err;
	}

	if((hdr.width | hdr.height) & 0xfff00000) {
		fprintf(stderr, "%s: invalid or corrupted bitmap file\n", fname);
		goto err;
	}
	bpp = hdr.num_bits * hdr.num_planes;

	img->width = hdr.width;
	img->height = hdr.height;

	if(!(img->pixels = (unsigned char*)malloc(hdr.width * hdr.height * 3))) {
		fprintf(stderr, "%s: failed to allocate RGBA image (%dx%d)\n", fname,
				hdr.width, hdr.height);
		goto err;
	}


	switch(bpp) {
	case 8:
		if(convbmp8(img, &hdr, fp, fname) == -1) {
			goto err;
		}
		break;

	case 24:
		if(convbmp24(img, &hdr, fp, fname) == -1) {
			goto err;
		}
		break;

	default:
		fprintf(stderr, "%s: unsupported bit depth: %d\n", fname, bpp);
		goto err;
	}

	fclose(fp);
	return img;

err:
	if(img) {
		free(img->pixels);
		free(img);
	}
	if(fp) fclose(fp);
	return 0;

}

static int convbmp8(Image *img, struct bmphdr *hdr, FILE *fp, const char *fname)
{
	unsigned int i, j, pitch, imgsize;
	unsigned char *pixels, *src, *dst;
	unsigned char cmap[256 * 4];

	fseek(fp, hdr->hdrsize + 14, SEEK_SET);

	if(fread(cmap, sizeof cmap, 1, fp) <= 0) {
		fprintf(stderr, "%s: EOF while trying to read colormap\n", fname);
		return -1;
	}

	fseek(fp, hdr->dataoffs, SEEK_SET);

	pitch = (hdr->width + 3) & 0xfffffffc;
	imgsize = hdr->height * pitch;
	if(!(pixels = (unsigned char*)malloc(imgsize))) {
		fprintf(stderr, "%s: failed to allocate %dx%d image\n", fname, hdr->width, hdr->height);
		return -1;
	}
	if(fread(pixels, 1, imgsize, fp) != imgsize) {
		fprintf(stderr, "%s: EOF while reading image (%dx%d)\n", fname,
				hdr->width, hdr->height);
		free(pixels);
		return -1;
	}

	src = pixels;
	dst = (unsigned char*)img->pixels;
	for(i=0; i<hdr->height; i++) {
		for(j=0; j<hdr->width; j++) {
			unsigned char *col = cmap + (src[j] << 2);
			dst[0] = col[2];
			dst[1] = col[1];
			dst[2] = col[0];
			dst += 3;
		}
		src += pitch;
	}
	free(pixels);
	return 0;
}

static int convbmp24(Image *img, struct bmphdr *hdr, FILE *fp, const char *fname)
{
	unsigned int i, j, pitch, imgsize;
	unsigned char *pptr;

	fseek(fp, hdr->dataoffs, SEEK_SET);

	pitch = (hdr->width * 3 + 3) & 0xfffffffc;
	imgsize = hdr->height * pitch;

	if(fread(img->pixels, 1, imgsize, fp) != imgsize) {
		fprintf(stderr, "%s: EOF while reading image (%dx%d)\n", fname,
				hdr->width, hdr->height);
		return -1;
	}

	pptr = (unsigned char*)img->pixels;
	for(i=0; i<hdr->height; i++) {
		for(j=0; j<hdr->width; j++) {
			unsigned char tmp = pptr[0];
			pptr[0] = pptr[2];
			pptr[2] = tmp;
			pptr += 3;
		}
		pptr += pitch - hdr->width * 3;
	}
	return 0;
}

static unsigned char *fuckRgb;

// The fucking weirdness of blueish around fonts (wtf? drivers???)
static void hackFonts(Image *texData)
{
	const int size = texData->width * texData->height;

	fuckRgb = new unsigned char[size * 4];

	unsigned char *src = (unsigned char*)texData->pixels;
	unsigned char *dst = fuckRgb;
	for (int i=0; i<size; ++i) {
		unsigned char r = *src++;
		unsigned char g = *src++;
		unsigned char b = *src++;
		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
		if ((r+g+b)/3 < 64)
			*dst++ = 0;
		else
			*dst++ = 255;
	}
}

void LoadGLTextures()
{

	Image *TextureImage[ntex];
	memset(TextureImage,0,sizeof(void *)*1);

	TextureImage[0]=LoadBMP("Data/optimus.bmp");
	TextureImage[1]=LoadBMP("Data/newater0.bmp");
	TextureImage[2]=LoadBMP("Data/newater1.bmp");
	TextureImage[3]=LoadBMP("Data/lava0.bmp");
	TextureImage[4]=LoadBMP("Data/fonts.bmp");
	TextureImage[5]=LoadBMP("Data/star.bmp");
	TextureImage[6]=LoadBMP("Data/pisinawall.bmp");
	TextureImage[7]=LoadBMP("Data/pisinafloor.bmp");
	TextureImage[8]=LoadBMP("Data/wall1.bmp");

	glGenTextures(9, texture);
	glGenTextures(1, &texPolar);

	hackFonts(TextureImage[4]);

	for (int i=0; i<ntex; i++)
	{
		glBindTexture(GL_TEXTURE_2D, texture[i]);

		if (i!=4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[i]->width, TextureImage[i]->height, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[i]->pixels);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureImage[i]->width, TextureImage[i]->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fuckRgb);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}


		if (TextureImage[i])							// If Texture Exists
		{
			if (TextureImage[i]->pixels)					// If Texture Image Exists
			{
				free(TextureImage[i]->pixels);				// Free The Texture Image Memory
			}
			free(TextureImage[i]);						// Free The Image Structure
		}
	}

	delete(fuckRgb);
}

#include <stdio.h>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glaux.h>



const int ntex=9;
int ntexs=ntex;
int tex;

unsigned int png, png_alpha, jpeg;


GLuint texture[ntex];
GLuint texPolar;

// ======= GL Texture Loading =======

AUX_RGBImageRec *LoadBMP(char *Filename)
{
	FILE *File=NULL;
	File=fopen(Filename,"r");
	fclose(File);
	return auxDIBImageLoad(Filename);

}

static unsigned char *fuckRgb;

// The fucking weirdness of blueish around fonts (wtf? drivers???)
static void hackFonts(AUX_RGBImageRec *texData)
{
	const int size = texData->sizeX * texData->sizeY;

	fuckRgb = new unsigned char[size * 4];

	unsigned char *src = (unsigned char*)texData->data;
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

	AUX_RGBImageRec *TextureImage[ntex];
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
			glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[i]->sizeX, TextureImage[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[i]->data);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureImage[i]->sizeX, TextureImage[i]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, fuckRgb);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}


		if (TextureImage[i])							// If Texture Exists
		{
			if (TextureImage[i]->data)					// If Texture Image Exists
			{
				free(TextureImage[i]->data);				// Free The Texture Image Memory
			}
			free(TextureImage[i]);						// Free The Image Structure
		}
	}

	delete(fuckRgb);
}

#ifndef TEXTURELOAD_H_
#define TEXTURELOAD_H_

struct Image {
	int width, height;
	void *pixels;
};

Image *LoadBMP(const char *Filename);
void LoadGLTextures();

#endif	/* TEXTURELOAD_H_ */

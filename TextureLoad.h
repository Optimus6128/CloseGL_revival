#ifndef TEXTURELOAD_H_
#define TEXTURELOAD_H_

#ifndef WIN32
struct AUX_RGBImageRec {
	int sizeX, sizeY;
	void *data;
};
#endif

AUX_RGBImageRec *LoadBMP(const char *Filename);
void LoadGLTextures();

#endif	/* TEXTURELOAD_H_ */

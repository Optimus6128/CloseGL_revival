#ifndef OPENGL_H_
#define OPENGL_H_

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#endif	/* OPENGL_H_ */

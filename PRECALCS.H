void Precalculations();
void LoadObject();
void LoadObject2();
void BlobColors();

#define nblob 1024
#define NSHAPES 32

typedef struct Vector
{
    float x;
    float y;
    float z;
} Vector;

typedef struct TexCoords
{
	float u,v;
} TexCoords;

typedef struct Color
{
	unsigned char r,g,b,a;
} Color;

typedef struct Color3
{
	unsigned char r,g,b;
} Color3;

Vector CrossProduct(Vector v1, Vector v2);
float DotProduct(Vector v1, Vector v2);
Vector Normalize(Vector v);
Vector Meon(Vector v);
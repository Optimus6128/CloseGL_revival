#define gqx 64
#define gqy 64

#define pqx 128
#define pqy 128

#define wqx 128
#define wqy 128

#define spx 90
#define spy 90

typedef struct point2d
{
    float x;
    float y;
    float z;
} point2d;

void GC_Polar(int nfx);
void GC_Plasma();
void GC_Distort();
void GC_FlatGrid(point2d p0, point2d p1, point2d p2, point2d p3);
void GC_Cube(float hx);
void GC_Water();
void GC_Spherical();
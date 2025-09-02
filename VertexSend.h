typedef struct ssine
{
    float speed;
    float sinediv;
    float sinemul;
	float sineadd;
} ssine;


typedef struct flower
{
    float tmul1, tmul2;
    float angdiv1, angdiv2;
    float sinmul1, sinmul2;
	float rdiv, gdiv, bdiv;
	float xfp,yfp,zfp;
	float fsize;
} flower;


void VS_Blob(float x, float y, float z, unsigned char r, unsigned char g, unsigned char b, float bsize);
void VS_Blob_Begin();
void VS_Blob_End();
void VS_Distort();
void VS_FlatGrid();
void VS_FlatGridNew(int face);
//void VS_GridPoints3D();
void RenderLight();
void VS_Stars3d();
void VS_ObjectShow(int way);
void VS_Water(int texn, float px, float py);
void VS_Spherical();
void VS_Floor(float y, float xsize, float zsize, float sdiv);
void VS_Tail(float sizex, int sdx, ssine s0, ssine s1, float r, float g, float b);
void VS_CubeTest(point2d p0, point2d p1, point2d p2, point2d p3, float rcx, float rcy, float rcz, int cubeside);
point2d VS_RotatePoint(point2d p, float rpx, float rpy, float rpz);
void VS_Stars2d(float distance, float size, int i0, int i1);
void VS_Pisina(float height1, float height2);
void VS_PisinaGrid(float xsize, float ysize, float xpos, float ypos, float tscale, float height);
void VS_WannabeCaustics(int texn, float px, float py, float hgt);
void VS_Flower(flower flo);
void VS_SkyBox();
float VS_FontWrite(char c, float xcp, float ycp, float z);
void VS_TextWrite(char *text, float xtp, float ytp, float z);
float VS_FontWrite2(char c, float xcp, float ycp, float z);
void VS_TextWrite2(char *text, float xtp, float ytp, float z);
void VS_Fade1(unsigned char fdc, float bsize);
void VS_Fade2(unsigned char fdc, float bsize);
void VS_Tile(float xpos, float ypos, float blend);
void Precalculations();
void LoadObject();
void LoadObject2();
void BlobColors();

#define nblob 1024
#define NSHAPES 32

typedef struct vector
{
    float x;
    float y;
    float z;
} vector;

vector CrossProduct(vector v1, vector v2);
float DotProduct(vector v1, vector v2);
vector Normalize(vector v);
vector Meon(vector v);
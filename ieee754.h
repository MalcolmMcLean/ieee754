#ifndef ieee754_h
#define ieee754_h

double freadieee754(FILE *fp, int bigendian);
float freadieee754f(FILE *fp, int bigendian);
int fwriteieee754(double x, FILE *fp, int bigendian);
int fwriteieee754f(float x, FILE *fp, int bigendian);

#endif

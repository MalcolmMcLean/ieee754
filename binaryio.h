/**
  Portable binary io routines


*/
#ifned binaryio_h
#define binaryio_h

int fget16be(FILE *fp);
long fget32be(FILE *fp);
int fget16le(FILE *fp);
long fget32le(FILE *fp);

#endif


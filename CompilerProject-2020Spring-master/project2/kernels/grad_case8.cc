#include "../run2.h"
void grad_case8(float (&dB)[32],float (&dA)[2][16]) {
  float tmp[2][16];
  float ret[2][16];
  for (int a=0;a<2;a++){
    for (int b=0;b<16;b++){
      ret[a][b]=0;
      tmp[a][b]=0;
      tmp[a][b]=(tmp[a][b] + (((a * 16) + b) < 32? (((a * 16) + b) >= 0? dB[((a * 16) + b)]: 0): 0));
      ret[a][b]=(ret[a][b] + tmp[a][b]);
      dA[a][b]=ret[a][b];
    }
  }
}

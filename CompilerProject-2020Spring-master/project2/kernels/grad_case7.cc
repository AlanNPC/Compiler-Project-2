#include "../run2.h"
void grad_case7(float (&dB)[16][32],float (&dA)[32][16]) {
  float tmp[32][16];
  float ret[32][16];
  for (int j=0;j<32;j++){
    for (int i=0;i<16;i++){
      ret[j][i]=0;
      tmp[j][i]=0;
      tmp[j][i]=(tmp[j][i] + (j < 32? (j >= 0? (i < 16? (i >= 0? dB[i][j]: 0): 0): 0): 0));
      ret[j][i]=(ret[j][i] + tmp[j][i]);
      dA[j][i]=ret[j][i];
    }
  }
}

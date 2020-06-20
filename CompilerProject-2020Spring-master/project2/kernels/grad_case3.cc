#include "../run2.h"
void grad_case3(float (&B)[16][16], float (&dC)[4][16],float (&dA)[4][16]) {
  float tmp[4][16];
  float ret[4][16];
  for (int i=0;i<4;i++){
    for (int k=0;k<16;k++){
      ret[i][k]=0;
      tmp[i][k]=0;
      for (int j=0;j<16;j++){
        tmp[i][k]=(tmp[i][k] + ((j < 16? (j >= 0? (i < 4? (i >= 0? dC[i][j]: 0): 0): 0): 0) * (j < 16? (j >= 0? (k < 16? (k >= 0? B[k][j]: 0): 0): 0): 0)));
      }
      ret[i][k]=(ret[i][k] + tmp[i][k]);
      dA[i][k]=ret[i][k];
    }
  }
}

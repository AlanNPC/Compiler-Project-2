#include "../run2.h"
void grad_case5(float (&C)[32][32], float (&D)[4][32], float (&dA)[16][32],float (&dB)[16][32][4]) {
  float tmp[16][32][4];
  float ret[16][32][4];
  for (int i=0;i<16;i++){
    for (int k=0;k<32;k++){
      for (int l=0;l<4;l++){
        ret[i][k][l]=0;
        tmp[i][k][l]=0;
        for (int j=0;j<32;j++){
          tmp[i][k][l]=(tmp[i][k][l] + (((j < 32? (j >= 0? (i < 16? (i >= 0? dA[i][j]: 0): 0): 0): 0) * (j < 32? (j >= 0? (l < 4? (l >= 0? D[l][j]: 0): 0): 0): 0)) * (j < 32? (j >= 0? (k < 32? (k >= 0? C[k][j]: 0): 0): 0): 0)));
        }
        ret[i][k][l]=(ret[i][k][l] + tmp[i][k][l]);
        dB[i][k][l]=ret[i][k][l];
      }
    }
  }
}

#include "../run2.h"
void grad_case4(float (&B)[16][32], float (&C)[32][32], float (&dA)[16][32], float (&dB)[16][32],float (&dC)[32][32]) {
  float tmp[32][32];
  float ret[32][32];
  for (int i=0;i<16;i++){
    for (int k=0;k<32;k++){
      ret[i][k]=0;
      tmp[i][k]=0;
      for (int j=0;j<32;j++){
        tmp[i][k]=(tmp[i][k] + ((j < 32? (j >= 0? (i < 16? (i >= 0? dA[i][j]: 0): 0): 0): 0) * (j < 32? (j >= 0? (k < 32? (k >= 0? C[k][j]: 0): 0): 0): 0)));
      }
      ret[i][k]=(ret[i][k] + tmp[i][k]);
      dB[i][k]=ret[i][k];
    }
  }
  for (int k=0;k<32;k++){
    for (int j=0;j<32;j++){
      ret[k][j]=0;
      tmp[k][j]=0;
      for (int i=0;i<16;i++){
        tmp[k][j]=(tmp[k][j] + ((k < 32? (k >= 0? (i < 16? (i >= 0? B[i][k]: 0): 0): 0): 0) * (j < 32? (j >= 0? (i < 16? (i >= 0? dA[i][j]: 0): 0): 0): 0)));
      }
      ret[k][j]=(ret[k][j] + tmp[k][j]);
      dC[k][j]=ret[k][j];
    }
  }
}

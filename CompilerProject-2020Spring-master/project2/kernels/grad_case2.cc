#include "../run2.h"
void grad_case2(float (&A)[4][16], float (&dB)[4][16],float (&dA)[4][16]) {
  float tmp[4][16];
  float ret[4][16];
  for (int i=0;i<4;i++){
    for (int j=0;j<16;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + (((j < 16? (j >= 0? (i < 4? (i >= 0? dB[i][j]: 0): 0): 0): 0) * (j < 16? (j >= 0? (i < 4? (i >= 0? A[i][j]: 0): 0): 0): 0)) + ((j < 16? (j >= 0? (i < 4? (i >= 0? A[i][j]: 0): 0): 0): 0) * (j < 16? (j >= 0? (i < 4? (i >= 0? dB[i][j]: 0): 0): 0): 0))));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      dA[i][j]=ret[i][j];
    }
  }
}

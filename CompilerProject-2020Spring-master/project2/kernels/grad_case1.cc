#include "../run2.h"
void grad_case1(float (&B)[4][16], float (&dC)[4][16],float (&dA)[4][16]) {
  float tmp[4][16];
  float ret[4][16];
  for (int i=0;i<4;i++){
    for (int j=0;j<16;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + ((j < 16? (j >= 0? (i < 4? (i >= 0? dC[i][j]: 0): 0): 0): 0) * (j < 16? (j >= 0? (i < 4? (i >= 0? B[i][j]: 0): 0): 0): 0)));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      dA[i][j]=ret[i][j];
    }
  }
}

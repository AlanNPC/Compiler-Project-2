#include "../run2.h"
void grad_case10(float (&dA)[8][8],float (&dB)[10][8]) {
  float tmp[10][8];
  float ret[10][8];
  for (int i=0;i<10;i++){
    for (int j=0;j<8;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + ((((j < 8? (j >= 0? (i < 8? (i >= 0? dA[i][j]: 0): 0): 0): 0) / 3) + ((j < 8? (j >= 0? ((i - 1) < 8? ((i - 1) >= 0? dA[(i - 1)][j]: 0): 0): 0): 0) / 3)) + ((j < 8? (j >= 0? ((i - 2) < 8? ((i - 2) >= 0? dA[(i - 2)][j]: 0): 0): 0): 0) / 3)));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      dB[i][j]=ret[i][j];
    }
  }
}

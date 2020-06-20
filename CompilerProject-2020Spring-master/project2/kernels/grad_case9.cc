#include "../run2.h"
void grad_case9(float (&dB)[4][6],float (&dA)[4]) {
  float tmp[4];
  float ret[4];
  for (int i=0;i<4;i++){
    ret[i]=0;
    tmp[i]=0;
    for (int j=0;j<6;j++){
      tmp[i]=(tmp[i] + (j < 6? (j >= 0? (i < 4? (i >= 0? dB[i][j]: 0): 0): 0): 0));
    }
    ret[i]=(ret[i] + tmp[i]);
    dA[i]=ret[i];
  }
}

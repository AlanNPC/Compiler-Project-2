#include "../run.h"
void kernel_case8(float (&B)[8][16],float (&A)[8][2][16]) {
  float tmp[8][2][16];
  float ret[8][2][16];
  for (int i=0;i<8;i++){
    for (int j=0;j<2;j++){
      for (int k=0;k<16;k++){
        ret[i][j][k]=0;
        tmp[i][j][k]=0;
        tmp[i][j][k]=(tmp[i][j][k] + (k < 16? (k >= 0? (i < 8? (i >= 0? B[i][k]: 0): 0): 0): 0));
        ret[i][j][k]=(ret[i][j][k] + tmp[i][j][k]);
        A[i][j][k]=ret[i][j][k];
      }
    }
  }
}

#include "../run.h"
void kernel_case2(float (&A)[16][8]) {
  float tmp[16][8];
  float ret[16][8];
  for (int i=0;i<16;i++){
    for (int j=0;j<8;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + (j < 8? (j >= 0? (i < 16? (i >= 0? A[i][j]: 0): 0): 0): 0));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + 2);
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      A[i][j]=ret[i][j];
    }
  }
}

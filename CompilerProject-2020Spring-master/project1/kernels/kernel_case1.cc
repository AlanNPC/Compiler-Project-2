#include "../run.h"
void kernel_case1(float (&A)[32][16]) {
  float tmp[32][16];
  float ret[32][16];
  for (int i=0;i<32;i++){
    for (int j=0;j<16;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + 2);
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      A[i][j]=ret[i][j];
    }
  }
}

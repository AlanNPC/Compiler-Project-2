#include "../run.h"
void kernel_case7(float (&A)[32][16],float (&B)[16][32]) {
  float tmp[16][32];
  float ret[16][32];
  for (int i=0;i<16;i++){
    for (int j=0;j<32;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + (i < 16? (i >= 0? (j < 32? (j >= 0? A[j][i]: 0): 0): 0): 0));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      B[i][j]=ret[i][j];
    }
  }
}

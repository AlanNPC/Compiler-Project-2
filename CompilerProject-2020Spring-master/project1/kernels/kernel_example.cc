#include "../run.h"
void kernel_example(float (&B)[32][16], float (&C)[32][16],float (&A)[32][16]) {
  float tmp[32][16];
  float ret[32][16];
  for (int i=0;i<32;i++){
    for (int j=0;j<16;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + ((j < 16? (j >= 0? (i < 32? (i >= 0? C[i][j]: 0): 0): 0): 0) * (j < 16? (j >= 0? (i < 32? (i >= 0? B[i][j]: 0): 0): 0): 0)));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      A[i][j]=ret[i][j];
    }
  }
}

#include "../run.h"
void kernel_case3(int (&B)[16][32], int (&C)[16][32],int (&A)[16][32]) {
  int tmp[16][32];
  int ret[16][32];
  for (int i=0;i<16;i++){
    for (int j=0;j<32;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + (j < 32? (j >= 0? (i < 16? (i >= 0? B[i][j]: 0): 0): 0): 0));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + (j < 32? (j >= 0? (i < 16? (i >= 0? C[i][j]: 0): 0): 0): 0));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      A[i][j]=ret[i][j];
    }
  }
}

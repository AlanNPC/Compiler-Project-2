#include "../run.h"
void kernel_case5(float (&B)[16][32], float (&C)[32][32], float (&D)[16][32], float (&alpha), float (&beta),float (&A)[16][32]) {
  float tmp[16][32];
  float ret[16][32];
  for (int i=0;i<16;i++){
    for (int j=0;j<32;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + (j < 32? (j >= 0? (i < 16? (i >= 0? A[i][j]: 0): 0): 0): 0));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      tmp[i][j]=0;
      for (int k=0;k<32;k++){
        tmp[i][j]=(tmp[i][j] + (alpha * ((k < 32? (k >= 0? (i < 16? (i >= 0? B[i][k]: 0): 0): 0): 0) * (j < 32? (j >= 0? (k < 32? (k >= 0? C[k][j]: 0): 0): 0): 0))));
      }
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      A[i][j]=ret[i][j];
    }
  }
  for (int i=0;i<16;i++){
    for (int j=0;j<32;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + (j < 32? (j >= 0? (i < 16? (i >= 0? A[i][j]: 0): 0): 0): 0));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + (beta * (j < 32? (j >= 0? (i < 16? (i >= 0? D[i][j]: 0): 0): 0): 0)));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      A[i][j]=ret[i][j];
    }
  }
}

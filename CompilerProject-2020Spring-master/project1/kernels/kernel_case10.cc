#include "../run.h"
void kernel_case10(float (&B)[10][10],float (&A)[8][8]) {
  float tmp[8][8];
  float ret[8][8];
  for (int i=0;i<10;i++){
    for (int j=0;j<10;j++){
      ret[i][j]=0;
      tmp[i][j]=0;
      tmp[i][j]=(tmp[i][j] + ((((j < 10? (j >= 0? (i < 10? (i >= 0? B[i][j]: 0): 0): 0): 0) + (j < 10? (j >= 0? ((i + 1) < 10? ((i + 1) >= 0? B[(i + 1)][j]: 0): 0): 0): 0)) + (j < 10? (j >= 0? ((i + 2) < 10? ((i + 2) >= 0? B[(i + 2)][j]: 0): 0): 0): 0)) / 3));
      ret[i][j]=(ret[i][j] + tmp[i][j]);
      A[i][j]=ret[i][j];
    }
  }
}

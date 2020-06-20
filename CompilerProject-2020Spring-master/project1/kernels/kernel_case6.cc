#include "../run.h"
void kernel_case6(float (&B)[2][16][7][7], float (&C)[8][16][3][3],float (&A)[2][8][5][5]) {
  float tmp[2][8][5][5];
  float ret[2][8][5][5];
  for (int n=0;n<2;n++){
    for (int k=0;k<8;k++){
      for (int p=0;p<5;p++){
        for (int q=0;q<5;q++){
          ret[n][k][p][q]=0;
          tmp[n][k][p][q]=0;
          tmp[n][k][p][q]=(tmp[n][k][p][q] + (q < 5? (q >= 0? (p < 5? (p >= 0? (k < 8? (k >= 0? (n < 2? (n >= 0? A[n][k][p][q]: 0): 0): 0): 0): 0): 0): 0): 0));
          ret[n][k][p][q]=(ret[n][k][p][q] + tmp[n][k][p][q]);
          tmp[n][k][p][q]=0;
          for (int c=0;c<16;c++){
            for (int r=0;r<3;r++){
              for (int s=0;s<3;s++){
                tmp[n][k][p][q]=(tmp[n][k][p][q] + (((q + s) < 7? ((q + s) >= 0? ((p + r) < 7? ((p + r) >= 0? (c < 16? (c >= 0? (n < 2? (n >= 0? B[n][c][(p + r)][(q + s)]: 0): 0): 0): 0): 0): 0): 0): 0) * (s < 3? (s >= 0? (r < 3? (r >= 0? (c < 16? (c >= 0? (k < 8? (k >= 0? C[k][c][r][s]: 0): 0): 0): 0): 0): 0): 0): 0)));
              }
            }
          }
          ret[n][k][p][q]=(ret[n][k][p][q] + tmp[n][k][p][q]);
          A[n][k][p][q]=ret[n][k][p][q];
        }
      }
    }
  }
}

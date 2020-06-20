#include "../run2.h"
void grad_case6(float (&C)[8][16][3][3], float (&dA)[2][8][5][5],float (&dB)[2][16][7][7]) {
  float tmp[2][16][7][7];
  float ret[2][16][7][7];
  for (int n=0;n<2;n++){
    for (int c=0;c<16;c++){
      for (int a=0;a<7;a++){
        for (int b=0;b<7;b++){
          ret[n][c][a][b]=0;
          tmp[n][c][a][b]=0;
          for (int k=0;k<8;k++){
            for (int r=0;r<3;r++){
              for (int s=0;s<3;s++){
                tmp[n][c][a][b]=(tmp[n][c][a][b] + (((b - s) < 5? ((b - s) >= 0? ((a - r) < 5? ((a - r) >= 0? (k < 8? (k >= 0? (n < 2? (n >= 0? dA[n][k][(a - r)][(b - s)]: 0): 0): 0): 0): 0): 0): 0): 0) * (s < 3? (s >= 0? (r < 3? (r >= 0? (c < 16? (c >= 0? (k < 8? (k >= 0? C[k][c][r][s]: 0): 0): 0): 0): 0): 0): 0): 0)));
              }
            }
          }
          ret[n][c][a][b]=(ret[n][c][a][b] + tmp[n][c][a][b]);
          dB[n][c][a][b]=ret[n][c][a][b];
        }
      }
    }
  }
}

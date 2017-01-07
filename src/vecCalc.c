/*****************
  feature vector calc
  m.nakai 
  2015.02.11
******************/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

char * comMalloc(int);

#define max_size  2000         // max length of strings
#define N  20                  // number of closest words that will be shown
#define max_w  50              // max length of vocabulary entries

int main(int argc, char **argv) {
  FILE *f;
  char st1[max_size];
  char *bestw[N];
  char file_name[max_size], st[100][max_size];
  float dist, len, bestd[N], vec[max_size];
  long words, size, a, b, c, d, cn, bi[100];
  char ch;
  float *M;
  char *vocab;

  char record[4096],*pc; //nakai
  int  lens;             //nakai
  char ope;              //nakai

  if (argc < 2) {
    printf("Usage: ./distance <FILE>\nwhere FILE contains word projections in the BINARY FORMAT\n");
    return 0;
  }
  strcpy(file_name, argv[1]);
  f = fopen(file_name, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  fscanf(f, "%lld", &words);
  fscanf(f, "%lld", &size);
  vocab = (char *)comMalloc((long)words * max_w * sizeof(char));
  for (a = 0; a < N; a++) bestw[a] = (char *)comMalloc(max_size * sizeof(char));
  M = (float *)comMalloc((long)words * (long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  } 
#if 0 //nakai
  fgets(record,4096,f);
  for (b = 0; b < words; b++) {
    fgets(record,4096,f);
    record[strlen(record)-1]='\0';
    pc=strtok(record," ");
    strcpy(&vocab[b * max_w],pc);

    for(a=0; a < size; a++) {
      pc=strtok(NULL," ");
      M[a + b * size] = atof(pc);
    }
#else
  for (b = 0; b < words; b++) {
    a = 0;

    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < size; a++) {
      fread(&M[a + b * size], sizeof(float), 1, f);
    }
#endif
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);


  /* 対話の繰返し処理 */
  while (1) {
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    printf("Enter word or sentence (Only enter to break): ");

    a = 0;
    while (1) {
      st1[a] = fgetc(stdin);
      if ((st1[a] == '\n') || (a >= max_size - 1)) {
        st1[a] = 0;
        break;
      }
      a++;
    }
    if (a == 0 || st1[0] == '\0') break;  //nakai

    /* 構文解析 */
    cn = 0;
    b = 0;
    c = 0;

    while (1) {
      st[cn][b] = st1[c];
      b++;
      c++;
      st[cn][b] = 0;
      if (st1[c] == 0) break;
      if (st1[c] == ' ') {
        cn++;
        b = 0;
        c++;
      }
    }

    cn++;
    for (a = 0; a < cn; a++) {  /* 文字群数 */
      if(st[a][0] == '+' || st[a][0] == '-' || st[a][0] == '*' || st[a][0] == '/') {
        continue;
      }
      for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
      if (b == words) b = -1;
      bi[a] = b;
      printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
      if (b == -1) {
        printf("Out of dictionary word!\n");
        break;
      }
    }
    if (b == -1) continue;

    /* タイトル行 */
    lens =0;  //nakai
    for(a=0;a < words; a++) {
      if(lens < strlen(&vocab[a * max_w])) lens = strlen(&vocab[a * max_w]);
    }
    lens += 5;
    printf("\nWord");
    for(a = 0;a < lens-4; a++) printf(" ");
    printf("Cosine distance\n");
    for(a = 0;a < lens + 15; a++) printf("-");
    printf("\n");  
    /***************
     指定した文字群のvecの算出（正規化)
    ****************/
    for (a = 0; a < size; a++) vec[a] = 0;
    for (b = 0; b < cn; b++) {  /* 文字群数 */
      if(st[b][0] == '+' || st[b][0] == '-' || st[b][0] == '*' || st[b][0] == '/') {
        ope = st[b][0];
        if(ope == '+') {
          for(a = 0; a < size; a++) vec[a] += M[a + bi[b+1] * size];
        }
        else if(ope == '-') {
          for(a = 0; a < size; a++) vec[a] -= M[a + bi[b+1] * size];
        }
        else if(ope == '*') {
          for(a = 0; a < size; a++) vec[a] *= M[a + bi[b+1] * size];
        }
        else if(ope == '/') {
          for(a = 0; a < size; a++) vec[a] /= M[a + bi[b+1] * size];
        }
        b++;
      }
      else {
        for(a = 0; a < size; a++) vec[a] = M[a + bi[b] * size];
      }
      //if (bi[b] == -1) continue;
     
    }

    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];
    len = sqrt(len);
    for (a = 0; a < size; a++) vec[a] /= len;


    for (a = 0; a < N; a++) bestd[a] = -1;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    for (c = 0; c < words; c++) {
      a = 0;
      for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
      if (a == 1) continue;
      /* 距離の算出 */
      dist = 0;
      for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
      /* 最大N個の入替え */
      for (a = 0; a < N; a++) {
        if (dist > bestd[a]) {
          for (d = N - 1; d > a; d--) {
            bestd[d] = bestd[d - 1];
            strcpy(bestw[d], bestw[d - 1]);
          }
          bestd[a] = dist;
          strcpy(bestw[a], &vocab[c * max_w]);
          break;
        }
      }
    }
    /* 最大距離N個の結果表示 */    
    for (a = 0; a < N; a++) { //nakai
      printf("%s",bestw[a]);
      for(b=0;b < lens - strlen(bestw[a]);b++) printf(" ");
      printf("%f\n", bestd[a]);
    }
  }
  return 0;
}
/**********************
  共通メモリー割り当て
***********************/
char *comMalloc(size)
int size;
{
   char *p;
   p=(char *)malloc(size);
   if(p) memset(p,'\0',size); 
   else  printf("malloc0 領域は確保出来ません!!\n");
   return(p);
}
/*********************
  文字種別
*********************/
type_of( char c )
{
	int type;

	if ( c == 0 ) {
		type = -1; /* EOS */
	} else if ( strchr( " \t\n\r\a", c) != NULL ) {
		type = 0; /* SPACE */
	} else if (( c >= 'A' ) && ( c <= 'Z' )) {
		type = 1; /* Alphabet */
	} else if (( c >= 'a' ) && ( c <= 'z' )) {
		type = 1; /* Alphabet */
	} else if ( strchr( "@_", c) != NULL ) {
		type = 1; /* SPACE */
	} else if (( c >= '0' ) && ( c <= '9' )) {
		type = 2; /* Number */
	} else if ( c == '.' ) {
		type = 3; /* dot */
	} else if ( strchr( "\'`", c ) != NULL ) {
		type = 4; /* quote */
	} else if ( strchr( "=<>+*^-/!%|", c ) != NULL ) {
		type = 5; /* operator */
	} else if ( strchr( "\\", c ) != NULL ) {
		type = 7;
	} else if ( strchr( "#?", c ) != NULL ) {
		type = 8;
	} else if ( strchr( "{}[]()", c ) != NULL ) {
		type = 9;
	} else if ( strchr( ";", c ) != NULL ) {
		type = 10;
	} else if ( strchr( "\"", c ) != NULL ) {
		type = 11;  /* wquote */
    } else if ( strchr(",",c ) != NULL) {
        type = 12;  /* connma 20080811 for round 2引数対応 */
	} else {
		type = 99;
	}
	return ( type );
}
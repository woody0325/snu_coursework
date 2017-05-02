#include <stdio.h>
#include <stdlib.h> // exit()

int main(void) {

  FILE *in;
  int ch;


  // test.bin 이라는 이진파일을 읽기 전용으로 열기
  if ( (in = fopen("temp_0_0", "rb")) == NULL ) {
    fputs("파일 열기 에러!", stderr);
    exit(1);
  }


  // 이진 파일, 1바이트씩 읽어, 헥사로 출력
  while ( (ch = fgetc(in)) != EOF ) {
    printf("%02X ", ch);
  }


  fclose(in); // 파일 닫기
  return 0;
}

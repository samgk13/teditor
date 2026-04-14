#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct termios o_ted;

void disableRawMode(){
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &o_ted);
}

void enableRawMode(){

  tcgetattr(STDIN_FILENO, &o_ted);
  atexit(disableRawMode);

  struct termios raw = o_ted;
  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

}


int main(void){

  enableRawMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')printf("%s",&c);
  return 0;

}
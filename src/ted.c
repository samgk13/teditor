#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>

struct termios o_ted;

void disableRawMode(){
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &o_ted);
}

void enableRawMode(){

  tcgetattr(STDIN_FILENO, &o_ted);
  atexit(disableRawMode);

  struct termios raw = o_ted;
  raw.c_lflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
  raw.c_lflag &= ~(OPOST);
  raw.c_lflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

}

int main(void){

  enableRawMode();
  
  while (1){
    
    char c = '\0';
    read(STDIN_FILENO, &c, 1);
    
    if (iscntrl(c)){
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  }

  return 0;

}
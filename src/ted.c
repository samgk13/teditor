/* includes */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <termios.h>
#include <unistd.h>

/* data */

struct termios o_ted;

/* terminal */

void die(const char *s){
  perror(s);
  exit(1);
}

void disableRawMode(){
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &o_ted) == -1) die("tcsetattr");
}

void enableRawMode(){

  if (tcgetattr(STDIN_FILENO, &o_ted) == -1) die("tcsetattr");
  atexit(disableRawMode);

  struct termios raw = o_ted;
  raw.c_lflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
  raw.c_lflag &= ~(OPOST);
  raw.c_lflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");

}

/* initialize */

int main(void){

  enableRawMode();
  
  while (1){
    
    char c = '\0';
    
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    
    if (iscntrl(c)){
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  }

  return 0;

}
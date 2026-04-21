#include "ted.h"

/* terminal */

void die(const char *s){                            // The crash handler / emergency exit
  write(STDOUT_FILENO, "\x1b[2J", 4);               // \x1b[2J -> ESC [ 2 J -> Clear entire screen
  write(STDOUT_FILENO, "\x1b[H", 3);                // \x1b[H = ESC [ H = Move cursor to home position (1,1) - top left corner

  perror(s);                                        // Print the system error message corresponding to errno
  exit(1);                                          // Terminate the program with error code 1
}

void disableRawMode(){                              // Restore normal terminal behavior
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.o_ted) == -1) die("tcsetattr");      
}

void enableRawMode(){                               // Switch terminal to "raw" mode

  // In raw mode:
  //   - No Enter key required (input is immediate)
  //   - No echo (typed characters aren't shown by the terminal itself)
  //   - Ctrl-C doesn't kill the program
  //   - Arrow keys and special keys are sent as raw byte sequences
  
  if (tcgetattr(STDIN_FILENO, &E.o_ted) == -1) die("tcsetattr");  // Save current terminal settings to E.o_ted
  atexit(disableRawMode);                                         // Register disableRawMode to run automatically when program exits

  struct termios raw = E.o_ted;

  // Modify input flags (c_iflag)
  raw.c_lflag &= ~(BRKINT                      // Disable: Break condition causes SIGINT
                  |INPCK                       // Disable: Parity checking
                  |ISTRIP                      // Disable: Strip 8th bit
                  |ICRNL                       // Disable: Convert Carriage Return to Newline (Ctrl-M sends '\r' not '\n')
                  |IXON);                      // Disable: Software flow control (Ctrl-S / Ctrl-Q)

  // Modify output flags
  raw.c_oflag &= ~(OPOST);                     // Disable: Output processing (tabs, newlines not translated)
  
  // Modify control flags
  raw.c_cflag &= ~(CS8);                       // Enable: 8-bit character size (full ASCII range)
  
  raw.c_lflag &= ~(ECHO                        // Disable: Echo typed characters back to screen
                  |ICANON                      // Disable: Canonical mode (line buffering, wait for Enter)
                  |IEXTEN                      // Disable: Extended input processing (Ctrl-V handling)
                  |ISIG);                      // Disable: Signal generation (Ctrl-C, Ctrl-Z)

  // Configure read() timeout behavior
  raw.c_cc[VMIN] = 0;                          // Minimum bytes to read
  raw.c_cc[VTIME] = 1;                         // Timeout in deciseconds

  // Apply the modified settings to the terminal
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");

}

int editorReadKey(){                                     // Read a single keypress from the terminal
  
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1){
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  if (c == '\x1b'){
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if(read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

    // format: ESC [ ...
    if(seq[0] == '['){
      if (seq[1] >= '0' && seq[1] <= '9'){
        if (read(STDIN_FILENO,&seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~'){                                      // the tilde '~' that ends these sequences
          switch (seq[1]){
            case '1': return HOME_KEY;                           // ESC [ 1 ~ -> Home
            case '3': return DEL_KEY;                            // ESC [ 3 ~ -> Delete key
            case '4': return END_KEY;                            // ESC [ 4 ~ -> End
            case '5': return PAGE_UP;                            // ESC [ 5 ~ -> Page Up
            case '6': return PAGE_DOWN;                          // ESC [ 6 ~ -> Page Down
            case '7': return HOME_KEY;                           // Alternative Home
            case '8': return END_KEY;                            // Alternative End
          }
        }
      } else {
         // format: ESC [ letter
        switch (seq[1]){
          case 'A': return ARROW_UP;                             // ESC [ A -> Up arrow 
          case 'B': return ARROW_DOWN;                           // ESC [ B -> Down arrow
          case 'C': return ARROW_RIGHT;                          // ESC [ C -> Right arrow
          case 'D': return ARROW_LEFT;                           // ESC [ D -> Left arrow
          case 'H': return HOME_KEY;                             // ESC [ H -> Home (Alternative)
          case 'F': return END_KEY;                              // ESC [ F -> End (Alternative)
        }
      }
    } 
    // Format: ESC O ...  (Older terminal style, just incase)
    else if (seq[0] == 'O'){
      switch (seq[1]){
        case 'H': return HOME_KEY;                               // ESC O H = Home
        case 'F': return END_KEY;                                // ESC O F = End
      }
    }

    return '\x1b';
  } else {
    // Regular ASCII character (letters, numbers, punctuation, Backspace(127))
    return c;
  }
}

int getCursorPositer(int *rows, int *cols){                    // Ask the terminal for cursor's position
  
  char buf[32];                                                // Buffer for terminal response
  unsigned int i = 0;

  if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1){                                 // Read the response character by character until we see 'R' or buffer fills
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;            // Validate response format: must start with ESC [
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}

int getWindowSize(int *rows, int *cols){                                     // terminal dimensions (rows and columns)
  struct winsize ws;

  if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){         // TIOCGWINSZ -> terminal I/O control get window size

    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;     // if the TIOCGWINSZ function doesnt work, just go to the max limits and check for cursor position
      return getCursorPositer(rows, cols);
  } 
  
  else {

    *cols =ws.ws_col;               // width terminal
    *rows = ws.ws_row;              // height terminal
    return 0;
  }
}
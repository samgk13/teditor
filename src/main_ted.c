#include "ted.h"

/* filetypes */

char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL };                                 // when opening a file, the editor checks if the filename ends with any of these 
char *C_HL_keywords[] = {                                                               // highlighting the keywords
  "switch", "if", "while", "for", "break", "continue", "return", "else", 
  "struct", "union", "typedef", "static", "enum", "class", "case",
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|", NULL
};

struct editorSyntax HLDB[] = {                                                          // HLDB -> Highlight Database
  {
    "c",
    C_HL_extensions,
    C_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
};

int hldb_entries = sizeof(HLDB) / sizeof(HLDB[0]);                     // calculate number of entries in HLDB, result: number of syntax definitions available


/* global */
struct editorConfig E;

/* initialized */
void initEditor(){

  // cursor starts at top-left corner
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;  // no tabs to expand yet, so 0

  // viewport starts unscrolled 
  E.rowoff = 0;
  E.coloff = 0;

  // document starts empty
  E.numrows = 0;
  E.row = NULL;
  E.dirty = 0;
  E.filename = NULL;

  // status bar starts empty
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;

  // no syntax highlighting until a file is opened
  E.syntax = NULL;

  // get terminal dimensions from the OS
  if(getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");

  // rows at the bottom for UI elements
  E.screenrows -= 2;
}

int main(int argc, char *argv[]){

  // Switch terminal to raw mode
  // This disables line buffering, echo, and Ctrl-C handling
  // Every keypress is sent directly to the editor immediately
  enableRawMode();
  
  // Initialize the editor state
  // Sets all fields to zero/NULL and queries terminal dimensions
  initEditor();

  // If a filename was provided as command-line argument, open it
  if (argc >= 2){
    editorOpen(argv[1]);
  }

  // Show the initial help message in the status bar
  editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
  
  while (1){

    // Draw everything: text rows, status bar, message bar
    editorRefreshScreen();
    
    // This function blocks until a key is pressed
    editorProcessKeypress();  
  }

  // The editor exits via die() or exit() when the user presses ctrl-q
  return 0;
}
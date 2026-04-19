#include "ted.h"

/* filetypes */

char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL };
char *C_HL_keywords[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else", 
  "struct", "union", "typedef", "static", "enum", "class", "case","int|", 
  "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|", NULL
};


struct editorSyntax HLDB[] = {
  {
    "c",
    C_HL_extensions,
    C_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
};

int hldb_entries = sizeof(HLDB) / sizeof(HLDB[0]);

/* global */
struct editorConfig E;

/* initialized */
void initEditor(){
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.numrows = 0;
  E.row = NULL;
  E.dirty = 0;
  E.filename = NULL;
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;
  E.syntax = NULL;

  if(getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
  E.screenrows -= 2;
}

int main(int argc, char *argv[]){

  enableRawMode();
  initEditor();

  if (argc >= 2){
    editorOpen(argv[1]);
  }

  editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
  
  while (1){
    editorRefreshScreen();
    editorProcessKeypress();  
  }

  return 0;
}
#ifndef TED_H
#define TED_H

/* includes */
#define _DEFAULT_SOURCE
#define _BCD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>      // character classification functions
#include <errno.h>      // error number definitions
#include <fcntl.h>      // file control options
#include <stdio.h>      // standard I/O
#include <stdarg.h>     // variable argument handling
#include <stdlib.h>     // standard library
#include <sys/ioctl.h>  // terminal I/O control
#include <string.h>     // string operations
#include <sys/types.h>  // system data types
#include <termios.h>    // terminal I/O attributes
#include <time.h>       // time functions
#include <unistd.h>     // POSIX operating system API: read(), write(), close(), STDIN_FILENO, STDOUT_FILENO

/* defines */
#define CTRL_KEY(k) ((k) & 0x1f) // masks the upper 3 bits, to make CTRL + "any key"
#define TED_VERSION "0.0.1"
#define TED_TAB_STOP 8           // number of visual colums tab expands to
#define TED_QUIT_TIMES 1         // number of times to press CTRL+q to quit

enum editorKey{
  BACKSPACE = 127,     // 127 -> DELETE
  ARROW_LEFT = 1000,   // To make sure that it doesn't clash with other ASCII KEY
  ARROW_RIGHT,         // 1001
  ARROW_UP,            // 1002
  ARROW_DOWN,          // 1003
  DEL_KEY,             // 1004 different from Backspace
  HOME_KEY,            // 1005
  END_KEY,             // 1006
  PAGE_UP,             // 1007
  PAGE_DOWN            // 1008
};

enum editorHighlight {     // colors/categories
  HL_NORMAL = 0,           // default state
  HL_COMMENT,              // 1
  HL_MLCOMMENT,            // 2
  HL_KEYWORD1,             // 3 -> primary keywords
  HL_KEYWORD2,             // 4 -> secondary keywords
  HL_STRING,               // 5
  HL_NUMBER,               // 6
  HL_MATCH                 // 7 -> search result highlight
};

#define HL_HIGHLIGHT_NUMBERS (1<<0)  // number bit flag, allowing for combinations
#define HL_HIGHLIGHT_STRINGS (1<<1)  // string bit flag

/* data */
#define ABF_INIT {NULL, 0}           // initial state of empty buffer {pointer,len}

struct abf {                         // builds screen content in memory before write
  char *b;
  int len;
};

struct editorSyntax {                // The syntax highlighting rules
  char *filetype;
  char **filematch;
  char **keywords;
  char *singleline_comment_start;
  char *multiline_comment_start;
  char *multiline_comment_end;
  int flags;
};

typedef struct erow {                 // represents single line of text in the text
  int idx;
  int size;
  int rsize;
  char *chars;
  char *render;
  unsigned char *hl;
  int hl_open_comment;
} erow;

struct editorConfig {
  // Cursor and viewport tracking
  int cx, cy;                       // cursor position x,y axis
  int rx;                           // rendered X position (counts the visual columns for proper screen placement)
  int rowoff;
  int coloff;

  // Terminal dimensions
  int screenrows;
  int screencols;

  // Document data
  int numrows;
  erow *row;
  int dirty;
  char *filename;

  // Status & UI
  char statusmsg[80];              // 80 -> same as the classic terminal width (not dynamic for now)
  time_t statusmsg_time;
  struct editorSyntax *syntax;

  // Terminal State
  struct termios o_ted;
};

extern struct  editorConfig E;

/* filetypes */
extern char *C_HL_extensions[];      // array of file extensions for C syntax highlightin
extern char *C_HL_keywords[];        // array of C keywords for highlighting
extern struct editorSyntax HLDB[];   // array of ALL supported syntax definitions
extern int hldb_entries;

/* prototypes */

// terminal.c
void die(const char *s);
void disableRawMode();
void enableRawMode();
int editorReadKey();
int getCursorPositer(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);

// features.c
int is_separator(int c);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight();
void editorFindCallback(char *query, int key);
void editorFind();


// operations.c
int editorRowCxToRx(erow *row, int cx);
int editorRowRxToCx(erow *row, int rx);
void editorUpdateRow(erow *row);
void editorInsertRow(int at, char *s, size_t len);
void editorFreeRow(erow *row);
void editorDelRow(int at);
void editorRowInsertChar(erow *row, int at, int c);
void editorRowAppendString(erow *row, char *s, size_t len);
void editorRowDelChar(erow *row, int at);
void editorInsertChar(int c);
void editorInsertNewline();
void editorDelchar();
void abAppend(struct abf *ab, const char *s, int len);
void abFree(struct abf *ab);

// io.c
char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave();
char *editorPrompt(char *prompt, void (*callback)(char *, int));
void editorMoveCursor(int key);
void editorProcessKeypress();
void editorScroll();
void editorDrawRows(struct abf *ab);
void editorDrawStatusBar(struct abf *ab);
void editorDrawMessageBar(struct abf *ab);
void editorRefreshScreen();
void editorSetStatusMessage(const char *fmt, ...);

// main_ted.c
void initEditor();

#endif
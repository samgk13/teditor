#ifndef TED_H
#define TED_H

/* includes */
#define _DEFAULT_SOURCE
#define _BCD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/* defines */
#define CTRL_KEY(k) ((k) & 0x1f) 
#define TED_VERSION "0.0.1"
#define TED_TAB_STOP 8
#define TED_QUIT_TIMES 1

enum editorKey{
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

enum editorHighlight {
  HL_NORMAL = 0,
  HL_COMMENT,
  HL_MLCOMMENT,
  HL_KEYWORD1,
  HL_KEYWORD2,
  HL_STRING,
  HL_NUMBER,
  HL_MATCH
};

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

/* data */
#define ABF_INIT {NULL, 0}

struct abf {
  char *b;
  int len;
};

struct editorSyntax {
  char *filetype;
  char **filematch;
  char **keywords;
  char *singleline_comment_start;
  char *multiline_comment_start;
  char *multiline_comment_end;
  int flags;
};

typedef struct erow {
  int idx;
  int size;
  int rsize;
  char *chars;
  char *render;
  unsigned char *hl;
  int hl_open_comment;
} erow;

struct editorConfig {
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  int numrows;
  erow *row;
  int dirty;
  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
  struct editorSyntax *syntax;
  struct termios o_ted;
};

extern struct  editorConfig E;

/* filetypes */
extern char *C_HL_extensions[];
extern char *C_HL_keywords[];
extern struct editorSyntax HLDB[];
extern int hldb_entries;

/* prototypes */
void die(const char *s);
void disableRawMode();
void enableRawMode();
int editorReadKey();
int getCursorPositer(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);
int is_separator(int c);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight();
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
char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave();
void editorFindCallback(char *query, int key);
void editorFind();
char *editorPrompt(char *prompt, void (*callback)(char *, int));
void editorMoveCursor(int key);
void editorProcessKeypress();
void editorScroll();
void editorDrawRows(struct abf *ab);
void editorDrawStatusBar(struct abf *ab);
void editorDrawMessageBar(struct abf *ab);
void editorRefreshScreen();
void editorSetStatusMessage(const char *fmt, ...);
void initEditor();

#endif
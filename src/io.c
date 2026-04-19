#include "ted.h"

/* file i/o */
char *editorRowsToString(int *buflen){
  int totlen = 0;
  int j;
  
  for ( j = 0; j < E.numrows; j++)
    totlen += E.row[j].size + 1;
  
  *buflen = totlen;

  char *buf = malloc(totlen);
  char *p = buf;
  
  for (j = 0; j < E.numrows; j++){
    memcpy(p, E.row[j].chars, E.row[j].size);
    p += E.row[j].size;
    *p = '\n';
    p++;
  }

  return buf;
}

void editorOpen(char *filename){
  free(E.filename);
  E.filename = strdup(filename);

  editorSelectSyntaxHighlight();

  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char *line = NULL;
  ssize_t linecap = 0;
  ssize_t linelen;

  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) 
      linelen--;
    editorInsertRow(E.numrows, line, linelen);
  }

  free(line);
  fclose(fp);
  E.dirty = 0;
}

void editorSave(){

  if (E.filename == NULL){
    E.filename = editorPrompt("Save as: %s (ESC to cancel)", NULL);
    
    if (E.filename == NULL) {
      editorSetStatusMessage("Save aborted");
      return;
    }
    editorSelectSyntaxHighlight();
  }

  int len;
  char *buf =editorRowsToString(&len);

  int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1) {
    if (ftruncate(fd, len) != -1){
      if (write(fd, buf, len) == len){
        close(fd);
        free(buf);
        E.dirty = 0;
        editorSetStatusMessage("%d bytes written to disk", len);
        return;
      }
    }
    close(fd);
  }
  
  free(buf);
  editorSetStatusMessage("Can't save.... I/O error: %s", strerror(errno));
}

/* input */
char *editorPrompt(char *prompt, void (*callback)(char *, int)){

  size_t bufsize = 128;
  char *buf = malloc(bufsize);
  
  size_t buflen = 0;
  buf[0] = '\0';
  
  while (1) {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen();
    
    int c = editorReadKey();
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE){
      
      if(buflen != 0) buf[--buflen] = '\0';

    } else if(c == '\x1b'){

      editorSetStatusMessage("");
      if (callback) callback(buf, c);
      free(buf);
      return NULL;
    
    } else if (c == '\r') {

      if (buflen != 0) {
        editorSetStatusMessage("");
        if (callback) callback(buf, c);
        return buf;
      }

    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }

    if (callback) callback(buf, c);
  }
}

void editorMoveCursor(int key) {
  erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

  switch(key) {
    case ARROW_LEFT:
      if (E.cx != 0){
        E.cx--;
      } else if (E.cy > 0){
        E.cy--;
        E.cx = E.row[E.cy].size;
      }
      break;
    case ARROW_RIGHT:
      if (row && E.cx < row->size){  
        E.cx++;
      } else if (row && E.cx == row->size){
        E.cy++;
        E.cx = 0;
      }
      break;
    case ARROW_UP:
      if (E.cy != 0){
        E.cy--;
      }  
      break;
    case ARROW_DOWN:
      if (E.cy != E.numrows ){
        E.cy++;
      }  
      break;
  }

  row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  int rowlen = row ? row->size : 0;

  if (E.cx > rowlen) {
    E.cx = rowlen;
  }
}

void editorProcessKeypress(){
  static int quit_times = TED_QUIT_TIMES;

  int c = editorReadKey();

  switch (c) {
    
    case '\r':
      editorInsertNewline();
      break;

    case CTRL_KEY('q'):

      if(E.dirty && quit_times > 0){
        editorSetStatusMessage("Warning... File has UNSAVED CHANGES. "
          "Press CTRL-Q %d more time to quit", quit_times);
        quit_times--;
        return;
      }  


      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
    
      exit(0);
      break;

    case CTRL_KEY('s'):
      editorSave();
      break;
    
    case HOME_KEY:
      E.cx = 0;
      break;

    case END_KEY:
      if (E.cy < E.numrows) E.cx = E.row[E.cy].size;
      break;

    case CTRL_KEY('f'):
      editorFind();
      break;
    
    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL_KEY:  
      if (c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
      editorDelchar();
      break;
    
    case PAGE_UP:
    case PAGE_DOWN:
      {

        if(c == PAGE_UP){
          E.cy = E.rowoff;
        } else if (c == PAGE_DOWN){
          E.cy = E.rowoff + E.screenrows - 1;
          if (E.cy > E.numrows) E.cy = E.numrows;
        }

        int times = E.screenrows;
        while(times--)
          editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
      break;

      case ARROW_UP:
      case ARROW_DOWN:
      case ARROW_LEFT:
      case ARROW_RIGHT:
        editorMoveCursor(c);
        break;
      
      case CTRL_KEY('l'):
      case '\x1b':
        break;
      
      default:
        editorInsertChar(c);
        break;
  }

  quit_times = TED_QUIT_TIMES;
}

/* output */
void editorScroll(){
  
  E.rx = 0;
  if(E.cy < E.numrows){
    E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
  }

  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }
  if (E.cy >= E.rowoff + E.screenrows){
    E.rowoff = E.cy - E.screenrows + 1;
  }
  if (E.rx < E.coloff){
    E.coloff = E.rx;
  }
  if (E.rx >= E.coloff + E.screencols) {
    E.coloff = E.rx - E.screencols + 1;
  }
}

void editorDrawRows(struct abf *ab){

  int y;
  for (y=0; y < E.screenrows; y++){
    int filerow = y + E.rowoff;
    if (filerow >= E.numrows){  
      if (E.numrows == 0 && y == E.screenrows / 3){
      
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
          "Ted-iter -- version %s", TED_VERSION);
      
        if (welcomelen > E.screencols) welcomelen = E.screencols;
        int padding = (E.screencols - welcomelen) / 2;
        if (padding) {
          abAppend(ab, "~",1);
          padding--;
        }

        while (padding--) abAppend(ab, " ", 1);
        abAppend(ab, welcome, welcomelen);
    
      } else {
        abAppend(ab, "~", 1);
      }
    } else {
      int len = E.row[filerow].rsize - E.coloff;
      if(len < 0) len = 0;
      if (len > E.screencols) len = E.screencols;

      char *c = &E.row[filerow].render[E.coloff];
      unsigned char *hl = &E.row[filerow].hl[E.coloff];
      int current_color = -1;
      int j;
      
      for (j = 0; j < len; j++) {
        if (iscntrl(c[j])) {

          char sym = (c[j] <= 26) ? '@' + c[j] : '?';
          abAppend(ab, "\x1b[7m", 4);
          abAppend(ab, &sym, 1);
          abAppend(ab, "\x1b[m", 3);

          if (current_color != -1) {
            
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
            abAppend(ab, buf, clen);
          }
        } else if (hl[j] == HL_NORMAL) {
          
          if (current_color != -1) {
            abAppend(ab, "\x1b[39m", 5);
            current_color = -1;
          }
          abAppend(ab, &c[j], 1);
        } else {
          int color = editorSyntaxToColor(hl[j]);
          if (color != current_color) {
            current_color = color;
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
            abAppend(ab, buf, clen);
          }
          abAppend(ab, &c[j], 1);
        }
      }
      abAppend(ab, "\x1b[39m", 5);    
    }

    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }

}

void editorDrawStatusBar(struct abf *ab){

  abAppend(ab, "\x1b[7m", 4);

  char status[80], rstatus[80];

  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", E.filename ? E.filename : "[No Name]", E.numrows, E.dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d", E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);

  if (len > E.screencols) len = E.screencols;

  abAppend(ab, status, len);

  while (len < E.screencols) {
    if (E.screencols - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    } else {
      abAppend(ab, " ", 1);
      len++;
    }
  }

  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(struct abf *ab){
  
  abAppend(ab, "\x1b[K", 3);

  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols) msglen = E.screencols;
  if (msglen && time(NULL) - E.statusmsg_time < 5) abAppend(ab, E.statusmsg, msglen);
}

void editorRefreshScreen(){
  editorScroll();

  struct abf ab = ABF_INIT;

  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawMessageBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
  abAppend(&ab, buf, strlen(buf));
  
  abAppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

void editorSetStatusMessage(const char *fmt, ...){
  
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}
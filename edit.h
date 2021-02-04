#ifndef _1989898_edit
#define _1989898_edit

#include "window.h"
void copyLine(cursor_xy_t cursor , cursor_max_t max_cursor,text_t text,char** buf );
void pasteLine(cursor_xy_t* cursor , cursor_max_t* max_cursor,text_t* text,char* buf );

void nextWord(cursor_xy_t* cursor,text_t text);
void prevWord(cursor_xy_t* cursor,text_t text);


#endif
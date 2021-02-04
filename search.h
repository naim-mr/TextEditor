#ifndef _SEARCH_20393930 
#define	_SEARCH_20393930 
#include "window.h"

typedef   int* shift_table_t;

typedef struct word_position_s{
	cursor_xy_t* pos;
	int nb_cursor;
}word_position_t;

void createShiftTab(shift_table_t* shift,  char* string);
void displayWord(char* word ,cursor_xy_t cursor,text_t t,word_position_t position);
void searchWord( word_position_t* word_pos,char* word,text_t text ,shift_table_t shift );
void mvWord(word_position_t position,cursor_xy_t* cursor,char* word, text_t text);
#endif
	

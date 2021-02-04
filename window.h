#ifndef _1029384756_w
#define _1029384756_w
#define BUFSIZE 8192 	
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>


#define hide_cursor(){write(0,"\x1b[?25l",6);}
#define display_cursor(){write(0,"\x1b[?25h",6);}
#define clear_screen() {write(STDOUT_FILENO, "\033c", 2);}
#define clear_line() {write(0,"\x1b[2K",4);}

// on définit des macros pour les touches du clavier 
#define UP  65
#define DOWN 66
#define LEFT 68
#define RIGHT 67
#define BACKSPACE 127
#define ESC 27
#define ENDLINE 70
#define LINE_BEGINING 72
#define LINEFEED 10
#define WHITESPACE 32
#define TAB 9
#define DELETE 126


//macros de couleur pour ma fonctioner void color(const char* color)
#define BLACK "30"
#define RED "31"
#define GREEN "32"
#define YELLOW "33"
#define BLUE "34"
#define PURPLE "35"
#define CYAN "36"
#define WHITE "97"



/* structures pour stocker la position courante du curseur et la position max qu'il a atteint afin de ne pas déplacer le curseur
la où il ne faut pas */
typedef struct cursor_xy_s{
	int col ;
	int row;
	int nb;
	int first_line;
}cursor_xy_t;



typedef struct cursor_max_s{
	int* col;
	int row;

}cursor_max_t;



static struct termios old; 
static struct termios new;
static struct winsize win;

static  cursor_xy_t cursor;
static  cursor_xy_t mouse;
static cursor_max_t max_cursor;

typedef struct node_char_s{
	char value; 
	int nb_node;
	struct node_char_t* next;
} node_char_t;

typedef struct text_s{
	int nb_row;
	int row_cursor;
	node_char_t** line_stk;
}text_t;

typedef struct editor_s{
	text_t text;
	cursor_xy_t cursor;
	cursor_xy_t mouse;
	cursor_max_t max_cursor;
}editor_t;


void parseText(char* buf,text_t* text,int nb,cursor_max_t* max_cursor);
void saveText(text_t t);
void quit(int code,const char msg[]);

void cursorColPlus(cursor_xy_t* cursor, cursor_max_t* max_cursor);
void cursorRowPlus(cursor_xy_t* cursor, cursor_max_t* max_cursor);
void cursorUp(int nb);
void cursorDown(int nb);
void cursorLeft(int nb);
void cursorRight(int nb);
void cursorXY(cursor_xy_t cursor);


void color(const char* color);

void mvLeft(cursor_xy_t* cursor,cursor_max_t* max_cursor);
void mvRight(cursor_xy_t* cursor, cursor_max_t* max_cursor);
void mvDown(cursor_xy_t* cursor, cursor_max_t* max_cursor,int maxr,text_t* text);
void mvUp(cursor_xy_t* cursor, cursor_max_t* max_cursor,text_t* text);

void delChar(cursor_xy_t* cursor,cursor_max_t * max_cursor,text_t* text );
void backspaceChar(cursor_xy_t* cursor ,cursor_max_t * max_cursor,text_t* text);
void lineFeed(cursor_xy_t* cursor,cursor_max_t* max_cursor,text_t* text,int maxr);
void insertInput(cursor_xy_t* cursor,cursor_max_t* max_cursor,text_t* text,char input,int maxr,int maxc);
int simpleCmd(int maxr,text_t* text,int fd);

node_char_t createNodeChar(char c,int nb);
void allocNode(node_char_t** node);
void insertChar(int row,int col,text_t* text,char input,cursor_max_t* max_cursor);

void displayPos(cursor_xy_t cursor);
void display_text(text_t text,cursor_xy_t cursor,cursor_max_t*  max_cursor,bool insertion);
void writeLine(node_char_t* line,int maxc);

void errorCmd(cursor_xy_t* cursor,int maxr);

void successCmd(cursor_xy_t* cursor,int maxr);
void printTab(int *tab,int n);
void printList(node_char_t*);
void pexit(const char msg[]);
void handleInput(cursor_xy_t* cursor,cursor_xy_t* mouse,cursor_max_t* max_cursor,text_t* text,int fd,struct termios old,struct termios new);

void freeText(text_t* del);
void freeLine(node_char_t** list);
text_t newText();

#include "edit.h"
#include "search.h"
#endif
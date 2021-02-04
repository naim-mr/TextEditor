
#ifndef _02022029299_w
#define _02022029299_w
#include "window.h"



typedef struct mouse_speed_s{
	float sum_col_up;
	float sum_col_down;
	float sum_row_up;
	float sum_row_down;
	float delta;
}mouse_speed_t;
int modifSpeedCmd(char c, mouse_speed_t* speed_control);
void setSpeed(mouse_speed_t* speed_control,float delta);
char charAtPos(cursor_xy_t cursor,text_t text);

void handleMouseInput(cursor_xy_t* cursor, cursor_max_t* max_cursor,cursor_xy_t* mouse,int fd_mouse,mouse_speed_t* speed_control,text_t text);
void mouseMv(int fd_mouse,cursor_xy_t* cursor,mouse_speed_t* speed_control,cursor_max_t* max_cursor,cursor_xy_t* mouse,int maxr,int maxc,text_t text);
#endif
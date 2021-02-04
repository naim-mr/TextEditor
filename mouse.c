
#include "mouse.h"

void setSpeed(mouse_speed_t* speed_control,float delta){
	if(delta<0) {
		write(0,"Delta doit être inférieur à 1",40);
		return;
	}
	speed_control->delta=delta;
	speed_control->sum_col_up=0;
	speed_control->sum_col_down=1;
	speed_control->sum_row_up=0;
	speed_control->sum_row_down=1;
}

	
/*	
J'ai utilisé ce lien pour le traitement des donées lu sur dev/input/mice 	
	   https://stackoverflow.com/questions/4855823/get-mouse-deltas-using-python-in-linux
				
*/
		


void handleMouseInput(cursor_xy_t* cursor,cursor_max_t* max_cursor, cursor_xy_t* mouse,int fd_mouse,mouse_speed_t* speed_control,text_t text){
		
		
				ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);


				int maxc= win.ws_col;
				int maxr = win.ws_row;
				cursor_xy_t temp_cursor;
				temp_cursor.row=mouse->row;
				temp_cursor.col=0;
				temp_cursor.first_line=0;

				cursorXY(temp_cursor);
				
				color(WHITE);
				clear_line();
				if(mouse->row+cursor->first_line <= max_cursor->row) writeLine(text.line_stk[(mouse->row+cursor->first_line)-1 ],maxc);
				hide_cursor();
				
			
				mouseMv(fd_mouse, cursor,speed_control,max_cursor,mouse,maxr,maxc,text)	;	
			
}



void mouseMv(int fd_mouse,cursor_xy_t* cursor,mouse_speed_t* speed_control,cursor_max_t* max_cursor,cursor_xy_t* mouse,int maxr,int maxc, text_t text){
		
		char buf[3];
		read(fd_mouse,buf,3);
		
		int button = (int)buf[0];
		int bLeft = button & 0x1;
		int bMiddle = (button & 0x4)  ; 
		int bRight = (button & 0x2) ;	
		int x = (int)buf[1];
		int y = (int)buf[2];


		if(x>0 && mouse->col < maxc){
						
			speed_control->sum_col_up += speed_control->delta;
			if(speed_control->sum_col_up >= 1){
				mouse->col++;
				speed_control->sum_col_up=0;
				}
			}
		if(x<0 && mouse->col>1){	
			speed_control->sum_col_down -= speed_control->delta;
			if(speed_control->sum_col_down <= 0){	
				mouse->col--;
				speed_control->sum_col_down=1;
			}
			
		}
		if(y>0 &&  (mouse->row>1))	{
			speed_control->sum_row_down -= speed_control->delta;
			if(speed_control->sum_row_down <= 0){
				mouse->row--;
				speed_control->sum_row_down=1;
			}
					
		}
		if(y<0 && mouse->row<maxr-1 ){
			speed_control->sum_row_up += speed_control->delta;
			if(speed_control->sum_row_up >= 1){
				mouse->row++;

				speed_control->sum_row_up=0;	
			}
		}
      

		if(bLeft==0 && bRight==0 && bMiddle==0  ){
				char c;
				if( mouse->row > max_cursor->row-1 || mouse->col> max_cursor->col[mouse->row-1] ){
					c=' ';	
				} else{
					c=charAtPos(*mouse,text);	
				} 
					cursorXY(*mouse);
					
					write(0,"\x1b[46;1m",7);
			   		write(0,&c,1);

			   		write(0,"\x1b[0m",4);

	 	}else {

					cursorXY(*mouse);
			   		if(  mouse->row> max_cursor->row) {
			   			cursor->row=max_cursor->row;
			   		}else{
			   			cursor->row=mouse->row+cursor->first_line;
			   		
			   		}	   	
			   		
			   		if(mouse->row+cursor->first_line <= max_cursor->row && mouse->col> max_cursor->col[(mouse->row+cursor->first_line)-1]) {
			   				cursor->col=max_cursor->col[(mouse->row+cursor->first_line)-1]+1;
			   		}else if(mouse->row+cursor->first_line > max_cursor->row && mouse->col> max_cursor->col[max_cursor->row-1]){
			   			cursor->col=max_cursor->col[max_cursor->row-1]+1;
			   		}else{
			   			cursor->col=mouse->col;
			   		}
			   		
		   		
		   		

		}
}






char charAtPos(cursor_xy_t cursor,text_t text){
	int col = cursor.col;
	int i=col;	
    node_char_t* line = text.line_stk[cursor.row-1];
	while(i>1 && line->next!=NULL){
		
		line=(node_char_t*)line->next;
		i--;
	}
	return line->value;
}


/* Vérifie la commande qui modifie la vitesse de la souris 
	Je ne commence pas à 0.1 car à chaque mouvement de souris je réeccris une ligne et si la souris est trop lente on voit des ligne apparaitre et réapparaitre 
*/

int modifSpeedCmd(char c, mouse_speed_t* speed_control){
		int ret=1;
		switch(c){
			case '1':
				
				setSpeed(speed_control,0.5);
				break;

			case '2':
				setSpeed(speed_control,0.55);
			    break;
			case '3':
				setSpeed(speed_control,0.6);
				break;
			case '4':
				setSpeed(speed_control,0.65);
			    break;
			case '5':
				setSpeed(speed_control,0.70);			
				break;
			case '6':
				setSpeed(speed_control,0.75);
			    break;
			case '7':
				setSpeed(speed_control,0.80);
				break;
			case '8':
				setSpeed(speed_control,0.85);
			    break;
			case '9':
				setSpeed(speed_control,0.9);
				break;
			default:
				ret=0;
				break;

		}
	return ret;
}

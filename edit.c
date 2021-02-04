
#include "edit.h"

void copyLine(cursor_xy_t cursor , cursor_max_t max_cursor,text_t text,char** buf ){

	int col=cursor.col;
	int end_line= max_cursor.col;
	int i=1;	

	node_char_t* node= text.line_stk[cursor.row-1];

	while(i<col){
		node=(node_char_t*)node->next;
		i++;

	}
	int j=0;
	while(i<end_line && node!=NULL ){
		(*buf)[j]=node->value;
		j++;
		i++;
		node=(node_char_t*)node->next;
	}

	(*buf)[j]='\0';
	
}


void pasteLine (cursor_xy_t* cursor , cursor_max_t* max_cursor,text_t* text,char* buf ){



	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	int maxc_term= win.ws_col;
	int maxr_term= win.ws_row;
	int col=cursor->col;
	int maxc_cur= max_cursor->col[cursor->row-1];
	int row = cursor->row-1;
	text->nb_row++;
	if((text->line_stk=(node_char_t**)realloc(text->line_stk,(text->nb_row+1)*sizeof(node_char_t*)))==NULL) quit(EXIT_FAILURE,"realloc a échoué");
	for(int i=text->nb_row-1;i>row;i--){
			(text->line_stk)[i]=(text->line_stk)[i-1];
	}
	/*Allocation de la nouvelle liste de caractère , et ajout de chaque noeud */
	if(( ((text->line_stk)[row]) =(node_char_t*)malloc(sizeof(node_char_t))  )==NULL) quit(EXIT_FAILURE,"Réalloc a échoué\n");
		node_char_t* pos=(text->line_stk)[row];
		
		for(int temp=0;buf[temp]!='\0';temp++){
			if(buf[temp+1]=='\0'){
				*(pos)=createNodeChar(buf[temp],temp+1);
						pos->next=NULL;
			}else {
			*(pos)=createNodeChar(buf[temp],temp+1);
			 if(((pos->next)= (struct node_char_t*)malloc(sizeof(node_char_t)))==NULL)quit(EXIT_FAILURE,"Malloc a échoué");
			pos=(node_char_t*)pos->next;
			}


		}
		text->line_stk[text->nb_row]=NULL;
		cursorRowPlus(cursor,max_cursor);
		cursor->col=1;
}




void nextWord(cursor_xy_t* cursor,text_t text){
	int i=1;
	int col= cursor->col;
	node_char_t* node= text.line_stk[cursor->row-1];
	while(i<col && node!=NULL){
		node=(node_char_t*)node->next;
		i++;

	}

	int j=0;
	while(node!=NULL && node->value!=' ' && node->value!='\n' && node->value!='\r'){

			node=(node_char_t*)node->next;
			j++;

	}
	while(node->value==' ' && node->value!='\n' && node->value!='\r'){
			node=(node_char_t*)node->next;
			j++;
	}

	if(node->value=='\n' || node->value=='\r') return ;
	cursor->col+= j;
	cursorXY(*cursor);


}


void prevWord(cursor_xy_t* cursor,text_t text){
	int i=1;
	int col= cursor->col;
	int prevcol=1;
	bool word;
	node_char_t* node= text.line_stk[cursor->row-1];
	if(node->value==' ') word=false;
	while(i<col && node!=NULL){
		if(node->value!=' ' && !word){
			prevcol=i;
			word=true;
		}
		if(node->value==' ') word=false;

		node=(node_char_t*)node->next;
		i++;

	}
	cursor->col= prevcol;
	cursorXY(*cursor);
}





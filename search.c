#include "search.h"
 

 /* Pour comprendre le principe de l'algorithme de Knuth-Morris-Pratt 
 		j'ai utilisé la ressource suivante https://fr.wikipedia.org/wiki/Algorithme_de_Knuth-Morris-Pratt
	
 		 */




void createShiftTab(shift_table_t* shift, char* string){
	int len =(int) strlen(string)+1;
	if((*shift=(shift_table_t)malloc((len)*sizeof(int)))==NULL) quit(EXIT_FAILURE,"MAlloc a échoué") ;
	/* On intitie une première case qui nous permettra de gérer des problème de bord */
	char str[len];
	str[0]='\0';

	for(int k=1; k<len;k++) str[k]=string[k-1];
	
	(*shift)[0]=-1;
	
	/* On remplis le tableau de caractère */
	
	/* L'idée est de créer un tableau de décalage ou chaque caractère du mot à une case associée

		Ce décalage nous permettra de savoir lors de l'analyse du texte où replacer le curseur pour ne pas relire à plusieur reprise
		le même caractère
	*/

	int i=1;
	int j=0;
	bool isequal;
	while(i<len){
		isequal=( str[j]== str[i]); // c'est là où la première case prend tout son sens en effet le premier décalage est logiquement toujorus 0
		if(isequal){
			j++;
			(*shift)[i]=j-1;
			i++;
		}else if(j>1){

			j=(*shift)[j-1];
		}else {
			(*shift)[i]=0;
			i++;
			j=1;
		}

	

	}

}

/* 
	L'idée est de créer une structure word_position_t avec avec un tableau de curseur dès qu'on trouve un mot on stock le curseur dans le tableau 


*/
void searchWord( word_position_t* position,char* word,text_t text ,shift_table_t shift ){
	
	if(text.nb_row==0)return ;
	
	int len=strlen(word);
	int nb_char_equal=0;
	int temp_int=0;

	
	node_char_t* node;
	node_char_t* head=NULL;
	
	/* Allocation du tableau */
	if((position->pos=(cursor_xy_t*)calloc(1,sizeof(cursor_xy_t)))==NULL)quit(EXIT_FAILURE,"MAlloc a échoué");
	
	/* Dimension du terminal */
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	int maxr= win.ws_row;	
	int maxc=win.ws_col;
	/* Nous permettra de stocker des curseurs dans position */

	cursor_xy_t temp_cursor;
	
	int j=0;
	int k=0;
	
	int col=1;
	int last_shift=0;
	int i=0;
	
	int check;
	bool next_line=true;
	/* On parcourt tout le text */
	while( temp_int < text.nb_row){
		
		if(next_line){
			node=text.line_stk[temp_int];
			next_line=false;
		}	
		while( node!=NULL && node->value==word[j]  && word[j]!='\0'){
				j++;
				/* on stock le décalage du caractère courant par rapport au début du mot */
				last_shift=shift[j];
				
				i++;
				
				head=node;
				node=(node_char_t*)node->next;
				
		
		} 
		if(i<len)i++;

		/* on a parcouru le mot en entier donc il est dans le text à la position col / temp_int+1 (mon curseur commence à 1)  */
		if(word[j]=='\0'){
			
			temp_cursor.col=col;
			temp_cursor.row=temp_int+1;
			temp_cursor.first_line=0;
			
			if(temp_int >maxr)temp_cursor.first_line= temp_int;
			if((position->pos=(cursor_xy_t*)realloc(position->pos,(k+1)*sizeof(cursor_xy_t)))==NULL)quit(EXIT_FAILURE,"Realloc a échoué");
			position->pos[k]=temp_cursor;
			
			col =col+i;
			k++;
			
			nb_char_equal=0;
			
			
				
		} 
		if(node==NULL  ){
		/* On est sortie de la boucle car on est en fin de ligne , on passe à la ligne suivante */
			temp_int++;
			col=1;
			i=0;
			j=0;
			next_line=true;
			
		}else if(node->value!=word[i] && last_shift>0) {
				
				/* 
				On est sortie de la boucel car le caractère de la ligne est différent de celui du mot cherché ;
					mais on a un décalage donc on se place au caractère juste après celui qui nous a sortie de la boucle
					
				*/
				j=last_shift;
				col +=(i-j);
				node=head->next;
				
				i=0;
								
				
				last_shift=0;
		}else  {

			if(node!=NULL){


				if(word[j]!='\0'){

					col +=i;
					node=node->next;
				}
					
				j=0;
				i=0;
			}else {
				j=0;
				i=0;
				nb_char_equal=0;
				col=1;
				next_line=true;
				temp_int++;
			}

		}

		

		

	
		
		
		

	}
 position->nb_cursor=k;

}
/* Similaire à display text sauf que je mets des fonds jaunes derrière le mot cherché , 
		Pb majeur je n'ai pas réussi à bufferiser cette fonction ça  n'affichait pas bien la couleur */
void displayWord(char* word ,cursor_xy_t cursor,text_t t,word_position_t position)	{
	

	clear_screen();

	//on réccuppère les dimensions de notre fenêtre
	
	int temp_int=position.pos[0].row-1;
	node_char_t* node=NULL;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);


	int maxr= win.ws_row;
	int maxc= win.ws_col;

	cursor_xy_t pos_0 ,pos_max;
	pos_0.row=0;
	pos_0.col=0;
	pos_0.first_line=0;
	pos_max.row=maxr;
	pos_max.col=1;
	pos_max.first_line=0;

	/* Affichage pûrement esthétique dans l'interface en dernière ligne si le mode est :insertion */

		cursorXY(pos_max);
		if(write(0,"\x1b[2K",4)<0)pexit("write");
		
		if(write(0," Cmd: quitter: 'q' , suivant:'l', précèdent :'l'",50)<0)pexit("write");
		
		cursorXY(pos_0);
	
	int compteur=0;
	char car;
	displayPos(cursor);
	cursorXY(pos_0);
	

	maxr =maxr+temp_int;
	bool next_list=true;
	int col =0;
	int row=temp_int;
		char buf[BUFSIZE];
	int len_w=strlen(word); // longueur du mot
	int len=0; // Longueur du buffer
	while(row<maxr-1 && 	temp_int<t.nb_row ){
		if(next_list) node=(t.line_stk)[temp_int];
		while(col<maxc && node!=NULL){
			if(compteur< position.nb_cursor && row==position.pos[compteur].row-1 && col==position.pos[compteur].col-1){
					
				/* À cette position il y a le mot recherché donc on met un jaune */
				if(write(0,"\x1b[43;1m",7)<0)pexit("write");
				len=len+11+len_w;
				for(int k=0;k<=len_w;k++){
					col++;
					car= node->value;
					if(write(0,&car,1)<0)pexit("write");		
					node=(node_char_t*)node->next;
				}
				
		   		if(write(0,"\x1b[0m",4)<0)pexit("write");
				compteur++;
			}else{
				car= node->value;
				write(0,&car,1);
				buf[len]=car;	
				len++;
				col++;
				node=(node_char_t*)node->next;
			}

		}

		if(node!=NULL ) {
			len++;
			buf[len]='\r';
			len++;
			buf[len]='\n';
			
			next_list=false;
			row++;
			col=0;

		
		}else if (node==NULL){
			
			col=0;
			next_list=true;
			row++;
			temp_int++;

		}
		
	}


}
/* Permet de se déplacer vers l'avant et l'arrière avant j et l , q nous permet de quitter la boucle */
void mvWord(word_position_t position,cursor_xy_t* cursor,char* word, text_t text){
	char input;
	int i=0;
	
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	int maxr= win.ws_row;
	int maxc= win.ws_col;
	
	cursor_xy_t temp_cursor;
	
	word_position_t memory_pos= position;
	int*  top_page;

	
	int nb=1;	
	int shitf;
	bool next_page=false;
	if(position.nb_cursor==0)return;
	while(nb=read(0,&input,1) && input!='q'){

		if(input=='l' &&  i+1<position.nb_cursor && position.pos[i+1].row>maxr){
			cursor->row=position.pos[i+1].row;
			cursor->col=position.pos[i+1].col;
			cursor->first_line=position.pos[i+1].row-1;
			
			position.pos=position.pos+i+1;
			
			position.nb_cursor -= i+1;
			displayWord(word ,temp_cursor,text, position);
			
			i=0;
			next_page=true;
			temp_cursor.row=1;
			temp_cursor.col=position.pos[0].col;

			cursorXY(temp_cursor);
			
			maxr+=maxr;	

			
		}else if(input=='j' && i>0){

			cursor->row=position.pos[i-1].row;
			cursor->col=position.pos[i-1].col;
			temp_cursor.row=position.pos[i-1].row-position.pos[0].row;
			temp_cursor.col=position.pos[i-1].col;
			i--;
			cursorXY(temp_cursor);


		}else if(input=='l'  && i<position.nb_cursor-1){
			
				cursor->row=position.pos[i+1].row;
				temp_cursor.row=position.pos[i+1].row-position.pos[0].row+1;
				cursor->col=position.pos[i+1].col;
				temp_cursor.col=position.pos[i+1].col;	
					i++;
			cursorXY(temp_cursor);

		}

		
		
											
	}

	
}


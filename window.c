

#include "window.h"
#include "mouse.h"
#include "edit.h"
#include "search.h"

/* Routine d'exit avec message sur l'erreur standard */
void quit(int code,const char msg[]){
	int len=strlen(msg);
	if(write(2,msg,len)<0)exit(EXIT_FAILURE);
	exit(code);
}


/* Routine d'exit avec perror */
void pexit(const char msg[]){
	perror(msg);
	exit(EXIT_FAILURE);
}


/* Fonctions d'initialisations et d'allocations de mes structures */
node_char_t createNodeChar(char c,int nb){

	node_char_t node;
	node.value=c;
	node.next=NULL;
	return node;
} 

/* Aloue un noeud */
void allocNode(node_char_t** ptr){
	
	if(((*ptr)= (node_char_t*)malloc(sizeof(node_char_t)))==NULL)quit(EXIT_FAILURE,"Malloc a échoué");
	
}

/* Initialisation de la structure texte_t */
text_t newText(){
	
	text_t text;
	text.nb_row=0;
	text.line_stk=NULL;
	return text;
}


/* Me permet d'afficher les listes de caractères pour du débugages */
void printList(node_char_t*  l){
	if(l==NULL){
		return;
	}else {
		
		if(write(0,&(l->value),1)<0)quit(EXIT_FAILURE,"Write a échoué");
		if((write(0,"|",1)<0))quit(EXIT_FAILURE,"Write a échoué");
		return printList((node_char_t*) l->next);
	}
}



/*
 	Stock toutes les lignes du text dans une structure contenant un tableau où chaque case est une
		liste chaînée de caractère et son curseur
	 	buf corresponde au buffer dans lequel read a stocké les caractères 
		nb est le nombre de caractère lu 
		max_cursor contient le nombre de caractère max par ligne
*/
void parseText(char* buf,text_t* text,int nb,cursor_max_t* max_cursor){
	

	/* On alloue au moins un noeud */
	if((text->line_stk=(node_char_t**)calloc(1,sizeof(node_char_t*)))==NULL) quit(EXIT_FAILURE,"Malloc a échoué\n");
	text->nb_row=1;

	/*On réccupère les dimensions de notre terminal */
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	int maxc= win.ws_col;
	
	int i=0;
	int j=0;
	int k=0;
	
	char line[BUFSIZE];

	while(i<nb){
		
		/* On alloue une nouvelle ligne à partir du deuxième tour de la boucle */
		if(k>0){
			if(((text->line_stk)=(node_char_t**)realloc(text->line_stk,(k+1)*sizeof(node_char_t*)))==NULL) quit(EXIT_FAILURE,"Réalloc a échoué\n");
			if((max_cursor->col=(int*)realloc(max_cursor->col,(k+2)*sizeof(int)))==NULL) quit(EXIT_FAILURE,"Malloc a échoué\n");
			
		}	

		j=0;
		
		/* On stock les caractères de la ligne*/
		while(buf[i]!='\n' && buf[i]!='\r' && j<nb && j<maxc && i<nb){
			if(buf[i]==TAB) buf[i]=' '; // Je fais le ce choix discutable de transformer les TABULATIONS en espace simple 
			line[j]=buf[i];
			i++;
			j++;
		}

		
		
		if(j==0 && (buf[i]=='\n' || buf[i]=='\r')) (max_cursor->col[max_cursor->row-1])=1 ;
		else (max_cursor->col[max_cursor->row-1]) = j;

		
		/* On augmente i pour éviter une boucle infinit , et on ajoute u \r\n car on a masqué OPOST pour STDIN */
		if(buf[i]=='\n' || buf[i]=='\r' ){
			line[j]='\r';	
			j++;
			line[j]='\n';
			j++;
			i++;
		}		

		/*Allocation de la nouvelle liste de caractère , et ajout de chaque noeud */
		if(( ((text->line_stk)[k]) =(node_char_t*)malloc(sizeof(node_char_t))  )==NULL) quit(EXIT_FAILURE,"Réalloc a échoué\n");

		
						
		node_char_t* pos=(text->line_stk)[k];



		for(int temp=0;temp<j;temp++){
			
						
				if(temp+1==j){
					*(pos)=createNodeChar(line[temp],j);
					pos->next=NULL;
				}else {
					*(pos)=createNodeChar(line[temp],j);
					 if(  ((pos->next)= (struct node_char_t*)malloc(sizeof(node_char_t)))==NULL)quit(EXIT_FAILURE,"Malloc a échoué");
					pos=(node_char_t*)pos->next;
				}
				

		}
		

		k++;
		if (i<nb){
				text->nb_row++;
				max_cursor->row++;
		}
	}
		if(((text->line_stk)=(node_char_t**)realloc(text->line_stk,(k+1)*sizeof(node_char_t*)))==NULL) quit(EXIT_FAILURE,"Réalloc a échoué\n");
		(text->line_stk)[k]=NULL;
		
	

}




	

/* Fonction me permettant d'afficher la position du curseur dans l'interface de mon éditeur à la derniere ligne 
	comme dans vim
	Argument : cursor_xy_t cursor me permettant de revenir à la bonne position pour le curseur à chaque fois
				et D'afficher la position courante 
*/
void displayPos(cursor_xy_t cursor){

	/* On réccupère les dimensions de la fenêtre */
	
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	int maxr= win.ws_row;
	int maxc= win.ws_col;
    
	/* On déplace le curseur à la dernière */
    cursor_xy_t temp_pos;
    temp_pos.col=maxc-20;
    temp_pos.row=maxr;
    temp_pos.first_line=0;
	cursorXY(temp_pos);

	/* Affichage de la position courante */
	char last_line[10];
	sprintf(last_line,"%d,%d",cursor.row,cursor.col);
	long len= strlen (last_line);
	if(write(0,last_line,len)<0) pexit("write");
	
	/* On revient à la position courante */
	cursorXY(cursor);
}







/* Ici on parcours le tableau de ligne et les liste de caractère pour afficher le text
	Paramètre : text_t t : le text à afficher 
				cursor : le cursor courant;
				insertion : booléen signifiant si le mode est en insertion ou non 
	
*/
void display_text(text_t t,cursor_xy_t cursor,cursor_max_t*  max_cursor,bool insertion) {
	clear_screen();	

	/*On réccupère les dimensions du terminal */
	int temp_int=cursor.first_line;
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
	if(insertion){
		cursorXY(pos_max);
		if(write(0,"\x1b[2K",4	)<0) pexit("write");
		if(write(0,"--INSERTION--",14)<0) pexit("write");
		
		cursorXY(pos_0);
	}


	;
	displayPos(cursor);
	cursorXY(pos_0);

	if(t.line_stk==NULL)return;

	char car;
	maxr--;                   //Pour ne pas écrire sur la dernière ligne
	bool next_list=true;
	
	int col =0;
	int row=0;
	/* Descritpion :	
		Col me permet de savoir si j'ai ou pas atteint le nombre de colonne maximum dans le terminal
	    si je ne l'ai pas atteint et que node n'est pas NULL alors il me reste des caractère à afficher 
		À ce moment la on met next_list à false pour ne pas modifier node.
		Sinon on passe à la liste suivante dans le tableau
		et cela jusqu'à que l'on est atteint la taille max du tableau ou le nombre max de ligne -1 dans le terminal 
	*/	
	char* buf;
	if((buf=(char*)malloc(BUFSIZE))==NULL)quit(EXIT_FAILURE,"Malloc a échoué");
	int len=0;

	while(row<maxr && 	temp_int<t.nb_row ){
		if(next_list) node=(t.line_stk)[temp_int];
		
		while(col<maxc && node!=NULL){
			car= node->value;
			if(len>=BUFSIZE) {
				if((buf=(char*)realloc(buf,len+1))==NULL)quit(EXIT_FAILURE,"Malloc a échoué");
			}
			buf[len]=car;	
			len++;
			col++;
			node=(node_char_t*)node->next;
		}

		if(node!=NULL ) {
			len ++;
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
	color(WHITE);
	if(write(0,buf,len)<0)pexit("write");
	free(buf);
	
}



/* Ecris une ligne : elle ne fera pas plus de BUFSIZE caractère bien évidemment.. */	
void writeLine(node_char_t* line,int maxc){
	int i=0;
	char buf[BUFSIZE];
	while(i<maxc && line!=NULL){
		buf[i]=line->value;
		line=(node_char_t*)line->next;
		i++;
	}
	if(write(0,buf,i)<0) pexit("perror");
}







/* Incrémentation simple du curseur de ligne en vérifiant s'il faut incrémenter le cursor correspondant au maximum 
		de  ligne  */
void cursorRowPlus(cursor_xy_t* cursor, cursor_max_t* max_cursor){ 
		cursor->row++; 
		if(max_cursor->row<cursor->row) {
			if((max_cursor->col=(int*)realloc(max_cursor->col,(cursor->row)*sizeof(int)))==NULL)quit(EXIT_FAILURE,"Echec malloc ");
			max_cursor->col[cursor->row-1]=1;
			max_cursor->row=cursor->row;
		}
		

}



/* Incrémentation simple du curseur de colonne en vérifiant s'il faut incrémenter la case correspondant au maximum 
	de caractère/colonne utilisée sur la ligne  */

void cursorColPlus(cursor_xy_t* cursor, cursor_max_t* max_cursor){ 
	cursor->col++; 
	if(max_cursor->col[cursor->row-1]<cursor->col){
		max_cursor->col[cursor->row-1]=cursor->col;

	}else {
		max_cursor->col[cursor->row-1]++;		
	}
}

/* Pour factoriser */
void cursor_f (int nb ,char c) {
	char cursor[5];
	sprintf(cursor,"\x1b[%d%c",nb,c);
	if(write(0,cursor,4)<0) pexit("write");	

}

/* Fonction déplaçant le curseur à une position (col,row) */
void cursorXY(cursor_xy_t cursor){
	int row;
	/* nombre de décimal de row et col*/
	int dc_r,dc_c;
	dc_r=1;
	dc_c=1;
	

	int rest_r,rest_c;
	/* Important cursor.first_line indique la ligne du texte afficher en premier sur le terminal */
	row=cursor.row-cursor.first_line;

	rest_r=row;
	rest_c=cursor.col;
	/* Nombre de decimale de row */
	while((rest_r=rest_r/10)>=1){
		dc_r++;
	}
	//nombre de dc de col
	while((rest_c=rest_c/10)>=1){
		dc_c++;
	}
	char position[dc_r+dc_c+4];



	sprintf(position,"\x1b[%d;%dH",row,cursor.col);
	if(write(0,position,dc_r+dc_c+4)<0) pexit("error");		
		
	
	
}

/* Ici je n'ai pas eu besoin de gérer les nombre  supérieurs à 10 car j'utilise ces fonctions pour des déplacements de 1 uniquement */
void cursorUp(int nb){
	cursor_f(nb,'A');

}
void cursorDown(int nb){
	cursor_f(nb,'B');

}
void cursorRight(int nb){
	cursor_f(nb,'C');

}
void cursorLeft(int nb){
	cursor_f(nb,'D');

}


/* Fonction déplaçant le curseur vers le haut de 1*/
void mvUp(cursor_xy_t* cursor,cursor_max_t* max_cursor,text_t *text){

		/* On a déjà scroller le texte vers le bas , le curseur est eà la première ligne du terminal mais pas la première du text */
		if(cursor->row-cursor->first_line==1 && cursor->first_line>0)  {
			cursor->first_line--;
			cursor->row--;
			display_text(*text,*cursor,max_cursor,true);
			cursorXY(*cursor);
		}else if(cursor->row-cursor->first_line>1){

				cursor->row--;
				/* Si la position du cursuer est supérieur à la position max de la ligne qui précède 
					on place le cursor à la position max de la ligne précédente */
					
				if(cursor->col>max_cursor->col[cursor->row-1]){
					if(max_cursor->col[cursor->row-1]==0)cursor->col=1;
					else cursor->col=max_cursor->col[cursor->row-1];
					cursorXY(*cursor);
					displayPos(*cursor);
				}else {
					cursorUp(1);
					displayPos(*cursor);
				}
		}
					
						
}
/*Fonction déplaçant le cursuer vers la gauche de  1*/
void mvLeft(cursor_xy_t* cursor,cursor_max_t* max_cursor){

		if(cursor->col>0){
			cursor->col--;
			cursorLeft(1);
			
		}else if(cursor->row>1){
			cursor->row--;
			cursor->col=max_cursor->col[cursor->row-1];
			cursorXY(*cursor);

		}
		displayPos(*cursor);
}

/*	Déplacement du curseur vers le bas */
void mvDown(cursor_xy_t* cursor,cursor_max_t* max_cursor,int maxr,text_t* text){
	if((cursor->row-cursor->first_line>maxr-2)  && cursor->row<text->nb_row){
		cursor->first_line++;
		cursorRowPlus(cursor,max_cursor);
		display_text(*text,*cursor,max_cursor,true);
		cursorXY(*cursor);
	}else if((cursor->row<max_cursor->row) ){
		cursorRowPlus(cursor,max_cursor);
		if(cursor->col>max_cursor->col[cursor->row-1] ){
			if(max_cursor->col[cursor->row-1]==0)cursor->col=1;
			else cursor->col=max_cursor->col[cursor->row-1];
			cursorXY(*cursor);
			displayPos(*cursor);
		}else{
			cursorDown(1);
			displayPos(*cursor);
		}
		
	}
}

/*Fonction permettant d'aérer handle input
	On déplace le curseur à droite tant que le curseur ne depassé pas le maximum de la ligne courante	*/
void mvRight(cursor_xy_t* cursor, cursor_max_t* max_cursor){
	if(cursor->col<max_cursor->col[cursor->row-1]	){
		cursor->col++;
		cursorRight(1);
		displayPos(*cursor);
	}
}



void color(const char* color){
	
	char buffer[5];
	sprintf(buffer,"\x1b[%sm",color);
	if(write(0,buffer,5)<0)pexit("error");
}



/*
 Cette fonction nous permet d'implémenter la fonctionnalité suppr (insertion/edition) et x en edition 
  	On supprime le caractère à la position du curseur 
  	Cas : 1) premier caractère on a juste à libérer ce maillon et à liée la tête de la liste au maillon suivant 
 	  	  2) caracètre en milieu : on effectue sensiblement la même chose sauf que l'on doit toujours 
					stocker le maillon précédent 
			3) Fin de liste : 
				 On doit lier la liste précédente et d'autre sous cas particulier		

			Paramètre : cursor  :structure donnant la position courante 
						max_cursor : structure donnant la position maximum possible pour le curseur à chaque ligne
						text: structure de stockage du texte 

Les paramètres sont passés par référence car j'appelle cette fonction dans handleInput() qui a pour paramètre des réferences
ce sera donc plus simple pour moi par la suite. 
*/

void delChar(cursor_xy_t* cursor,cursor_max_t * max_cursor,text_t* text ){
	clear_screen();
	
	int col = cursor->col;
	int row = cursor->row-1;
	int max_col= max_cursor->col[row];
	int next_col_max=0;
	
	int i=col;

	/* Suppression en fin de liste il faut liée la liste suivante à cette liste */
	if(col==max_col ){
		
		node_char_t* del_2; 
		
		/* Denier caractère du texte */
		if(max_col==1 && cursor->row==max_cursor->row) {
			free(text->line_stk[row]);
			text->line_stk[row]=NULL;

		}
		

		/* La ligne suivante est vide */
		if(text->line_stk[row+1]!=NULL && text->line_stk[row+1]->value=='\n' && max_cursor->col[row+1]==1 ){

			freeLine(&(text->line_stk[row+1]));
			text->line_stk[row+1]=NULL;
			
			// J'ai eu des problèmes en incluant les deux ligne dans une même boucle donc j'ai fait deux cas ce qui a fonctionné..
			for(int i=row+1;i<text->nb_row-1;i++)text->line_stk[i]=text->line_stk[i+1];
			for(int i=row+1; i<max_cursor->row-1; i++)max_cursor->col[i]=max_cursor->col[i+1];
			
			max_cursor->row--;
			text->nb_row--;
			return;
		}
		/* On supprime une ligne vide */
		if(text->line_stk[row]!=NULL && text->line_stk[row]->value=='\n' && max_cursor->col[row]==1){
			
			freeLine(&(text->line_stk[row]));
			text->line_stk[row]=NULL;

			// J'ai eu des problèmes en incluant les deux ligne dans une même boucle donc j'ai fait deux cas ce qui a fonctionné..
			for(int i=row;i<text->nb_row-1;i++)text->line_stk[i]=text->line_stk[i+1];
			for(int i=row; i<max_cursor->row-1; i++)max_cursor->col[i]=max_cursor->col[i+1]; 
			
			max_cursor->row--;
			text->nb_row--;
			return;
		}

		node_char_t* head=text->line_stk[row];
		node_char_t* prec;
		node_char_t* del;
		
		/* On avance jusqu'au caractère à supprimer */
		while(i>0	 && head!=NULL){
			prec=head ;
			head=(node_char_t*)head->next;
			i--;

		}
		if(head !=NULL){
			prec->next=head->next;
			free(head);
			head=NULL;
		}

		/* Si la ligne courante n'est pas la dernière il faut concaténer la ligne actuelle row , avec la ligne suivante row+1
			et décaler le tableau de ligne vers la gauche */
		del=NULL;
		if(row<text->nb_row-1){
			if(prec->next) del=(node_char_t*)prec->next; 
			while(del && (del->value=='\r' || del->value=='\n')){	
		
			/* del_2 est déclaré car sinon free retourne un msg erreur dans valgrind en faisant free(del) */
			del_2=del;
			del=(node_char_t*)del->next;
			free(del_2);
		}
		
		next_col_max= max_cursor->col[row+1];
		prec->next=(struct node_char_t*)text->line_stk[row+1];
		text->line_stk[row+1]=NULL;
		
		for(int i=row+1;i<text->nb_row-1;i++) text->line_stk[i]=text->line_stk[i+1];
		for(int i=row+1; i<max_cursor->row-1; i++)max_cursor->col[i]=max_cursor->col[i+1];
		text->nb_row--;
		max_cursor->row--;

		}

	/* Suppression simple en début /milieu/fin de liste  */
	}else if(col==1){
		
		if(text->line_stk[row]){
			node_char_t* del =text->line_stk[row];	
			text->line_stk[row]=(node_char_t*)text->line_stk[row]->next;
	    	del->next=NULL;
	    	free(del);
	    	del=NULL;
	    }
	    max_col--;
	}else if(col>1){
		
		node_char_t* head=text->line_stk[row];
		node_char_t* prec; 
		while(i>1 && head!=NULL){
			prec=head ;
			head=(node_char_t*)head->next;
			i--;

		}

		if(head){
			prec->next=head->next;
			head->next=NULL;
			free(head);
		}			
		head=NULL;
		max_col--;
	}

	/* Nouveau nombre de caractère max sur la ligne */
	max_cursor->col[row]=max_col+next_col_max;	
}




/*	Fonction permettant de supprimer un caractère 
	Il faut distinguer deux cas ici aussi :
			- Si c'est en début de ligne il faut relier la fin de la ligne précédente à la ligne courante et puis supprimer
				la case de la ligne courante dans le tableau de la strucutre text (sous cas: ligne vide , ligne précèdente est vide)
			 - Si c'est après le premier caractère on a juste a effectué une suppression de noeud dans une liste 

Paramètre : cursor  :structure donnant la position courante 
			max_cursor : structure donnant la position maximum possible pour le curseur à chaque ligne
			text: structure de stockage du texte 

Les paramètres sont passés par référence car j'appelle cette fonction dans handleInput() qui a pour paramètre des réferences
ce sera donc plus simple pour moi par la suite. 
*/

void backspaceChar(cursor_xy_t* cursor ,cursor_max_t * max_cursor,text_t* text){
	
	/* cursor->col/row commencent à 1 */
	int col = cursor->col;
	int row = cursor->row-1;
	int maxcol=max_cursor->col[row];
	int i,j;
	

	if(col<0 || row<0){
		write(2,"Insert char :Les coordonnées doivent être positives\n",54);
		return;
	}

	if((col==1 || ((col==0||col==1) && row>=1))){
		/* Cas numéro 1 */
		
		/* On stock le nombre de caractère sur la ligne précédente et la ligne  précédente */
		int col_line= max_cursor->col[row-1]-1;
		node_char_t* line_to_link = text->line_stk[row-1];
		i=col_line;
		
		/* Sous cas : si la ligne qui précède ou courante la ligne courante est uniquement un '\n' on traite ce cas à part */
		if(text->line_stk[row]!=NULL && (text->line_stk[row]->value=='\n' ||text->line_stk[row]->value=='\r') &&max_cursor->col[row]==1 ){
			freeLine(&(text->line_stk[row]));
			text->line_stk[row]=NULL;
			for(int i=row;i<text->nb_row-1;i++)text->line_stk[i]=text->line_stk[i+1];
			for(int i=row; i<max_cursor->row-1; i++)max_cursor->col[i]=max_cursor->col[i+1];
			max_cursor->row--;
			text->nb_row--;
			cursor->col=max_cursor->col[row-1];
			cursor->row--;
			return;
		}
		if(text->line_stk[row-1]!=NULL && (text->line_stk[row-1]->value=='\n' ||text->line_stk[row-1]->value=='\r') && col_line+1==1 ){
			
			freeLine(&(text->line_stk[row-1]));
			text->line_stk[row-1]=NULL;
			for(int i=row-1;i<text->nb_row-1;i++)text->line_stk[i]=text->line_stk[i+1];
			for(int i=row-1; i<max_cursor->row-1; i++)max_cursor->col[i]=max_cursor->col[i+1];
			max_cursor->row--;
			text->nb_row--;
			if(col_line==0)col_line=1;
			cursor->col=col_line;
			cursor->row--;
			clear_screen();
			return;
		}
			

		node_char_t* del;
		node_char_t* del_2;
		
		/* On avance jusqu'au dernier maillon de la list pour la ligne précédente */
		while(i>1 && line_to_link->next !=NULL){	
			line_to_link=(node_char_t*)line_to_link->next;
			i--;
		}
		
		/* On supprime les \n \r qui se retrouve en fin de la ligne row-1 et en debut de la ligne row pour pouvoir
			bien relier les deux lignes */
		if(line_to_link)del=(node_char_t*)line_to_link->next;
			
		while(del && (del->value=='\r' || del->value=='\n')){	
			/* del_2 est déclaré car sinon free retourne un msg erreur dans valgrind en faisant free(del) */
			del_2=del;
			del=(node_char_t*)del->next;
			free(del_2);
		}

		if(del){
			line_to_link->next=(struct node_char_t*)del;
			del->next=(struct node_char_t*)text->line_stk[row];
		}else {
			line_to_link->next=(struct node_char_t*) text->line_stk[row];
		}
		text->line_stk[row]=NULL;			
		
		
		/* 
			Si la ligne n'est pas la dernière il faut modifier le tableau de la structure text et celui de max_cursor 
			en décalant toutes les cases vers la gauche et en mettant la dernière à NULL ou à -1 
			Voir rapport : Problème ici 
		*/

		if(text->nb_row>cursor->row){
			for( j=row;j<text->nb_row-1;j++){
				text->line_stk[j]=text->line_stk[j+1];
			}
			for(j=row;j<max_cursor->row-1;j++) max_cursor->col[j]=max_cursor->col[j+1];
			max_cursor->col[max_cursor->row-1]=-1;
			text->line_stk[text->nb_row-1]=NULL;
		}
		text->nb_row--;
		cursor->col=col_line+1;
		cursor->row--;
		max_cursor->col[row-1]= maxcol+ col_line;
		return;
	}

	/* Cas simple  il s'agit d'une suppression de maillon */
	/* On réduit de 1 pour bien supprimer le caracètre juste avant */
	i=col-1; 	
	node_char_t* list = text->line_stk[row];
	node_char_t* prec;
	if(list==NULL){
		return;
	}else {
		while((i>1 && list->next !=NULL) ||(i>0 && list->next !=NULL && col==2)){
			prec=list;
			list=(node_char_t*)list->next;
			i--;
		}
		if(prec) prec->next =list->next;	
		if(cursor->col==max_cursor->col[row])max_cursor->col[row]--;
		cursor->col--;
		return;
	}


}








/*	Fonction permettant d'insérer  un caractère 
	Il faut distinguer deux cas ici aussi :
			- Si c'est une insertion ligne ->'\n' 
			- Si c'est une insertion de caractère ce qui correspond alors à une insertion simple dans une liste chaînée

Paramètre : col: entier correspondant à la coloonne courante commençant à  1
			row: entier correspondant à la ligne courante commençant à  1
			max_cursor : structure donnant la position maximum possible pour le curseur à chaque ligne
			text: structure de stockage du texte 
			input : le caractère à insérer , \n si c'est un saut de ligne

Les paramètres sont passés par référence car j'appelle cette fonction dans handleInput() qui a pour paramètre des réferences
ce sera donc plus simple pour moi par la suite. 
*/

//row commence  à 1 et text->nb_row aussi
void insertChar(int row,int col,text_t* text,char input,cursor_max_t* max_cursor){
	
	if(input==TAB)input=' ';
	
	if(col<0 || row<0){	
		if(write(2,"Insert char :Les coordonnées doivent être positives\n",54)<0)quit(EXIT_FAILURE,"Write a échoué");
		return;
	}
	
	row--;
	int i=col;
	
	
	node_char_t* temp_node;	
	if(((temp_node)= (node_char_t*)malloc(sizeof(node_char_t)))==NULL)quit(EXIT_FAILURE,"Malloc a échoué");
	*temp_node=(createNodeChar(input,1));
	
	//On stock la ligne courante 	
	node_char_t* list = text->line_stk[row];
	node_char_t* list_2=text->line_stk[row];
	

	/* insertion dans une ligne vide */
	if(list==NULL){
		if((text->line_stk=(node_char_t**)realloc(text->line_stk,(text->nb_row+1)*sizeof(node_char_t*)))==NULL) quit(EXIT_FAILURE,"realloc a échoué");
		text->nb_row++;
		text->line_stk[row]=temp_node;
		text->line_stk[row+1]=NULL;
		return;
		
	}

	int nb_node=1;
	if(input=='\n'){
		
		/* On rajoute un \r car on a enlevé le masque OPOST , qui automatise la transformation NL -> CRNL */
		node_char_t* car_r;
		if(((car_r)= (node_char_t*)malloc(sizeof(node_char_t)))==NULL)quit(EXIT_FAILURE,"Malloc a échoué");
		*car_r=(createNodeChar('\r',nb_node));
		car_r->next=(struct node_char_t*)temp_node;
		
		/* Insertion au premier caractère doit juste faire descend la ligne vers le bas et conservé le premier caractère */
		if(col==1){
			text->nb_row++;
			if((text->line_stk=(node_char_t**)realloc(text->line_stk,(text->nb_row+1)*sizeof(node_char_t*)))==NULL) quit(EXIT_FAILURE,"realloc a échoué");

			for(int i=text->nb_row-1;i>row;i--){
					(text->line_stk)[i]=(text->line_stk)[i-1];
				}
			text->line_stk[row+1]=list;
			text->line_stk[row]= car_r;
			temp_node->next=NULL;
			text->line_stk[text->nb_row]=NULL;
			return;
		}

		/* On va jusqu'au noeud correspondant au caractère avant le \r\n */
		while(i>1 && list->next !=NULL){
			list=(node_char_t*)list->next;
			i--;
		}
		temp_node->next=list->next;
		list->next=(struct node_char_t*)car_r;
   
		/* Il faut incrémenter le tableau de la structure text et décaler chaque ligne vers la "droite" si c'est au milieu du text*/
		if(row<text->nb_row-1){
			
			text->nb_row++;
			if((text->line_stk=(node_char_t**)realloc(text->line_stk,(text->nb_row+1)*sizeof(node_char_t*)))==NULL) quit(EXIT_FAILURE,"realloc a échoué");

				for(int i=text->nb_row-1;i>row;i--){
					(text->line_stk)[i]=(text->line_stk)[i-1];
				}
				text->line_stk[row+1]=(node_char_t*) temp_node->next;
				text->line_stk[row]= list_2;
				temp_node->next=NULL;
				text->line_stk[text->nb_row]=NULL;
			}else if(row==text->nb_row-1){

				text->nb_row++;
				if((text->line_stk=(node_char_t**)realloc(text->line_stk,(text->nb_row+1)*sizeof(node_char_t*)))==NULL) quit(EXIT_FAILURE,"realloc a échoué");
				text->line_stk[text->nb_row-1]=(node_char_t*)temp_node->next;
				text->line_stk[text->nb_row]=NULL;
				temp_node->next=NULL;
			}
		return;
	     }

	/* Insertion simple */
	if(col==1 ){
		
		temp_node->next=(struct node_char_t*)list;
		text->line_stk[row]=temp_node;
		return;
	}else if(col>1){
		while(i>2 && list->next !=NULL){
			list=(node_char_t*)list->next;
			i--;
		}
		if(list->value=='\n' || list->value=='\r'){
			temp_node->value=list->value;
			list->value=input;

		}
		
		temp_node->next=list->next;
		list->next=(struct node_char_t*)temp_node;
	}


	



}

/* Read a lu sur l'entrée standard et à renvoyé \n 
	Cette fonction me permet d'aérer le code handleInput()
	Routine d'insertion d'une nouvelle ligne 
*/
void lineFeed(cursor_xy_t* cursor,cursor_max_t* max_cursor,text_t* text,int maxr){	
	// On stock le nombre de caractère après le saut pour le maximum de colonne de la nouvelle ligne
	int new_max_col=max_cursor->col[cursor->row-1]-(cursor->col);
	if(cursor->col==1)new_max_col++;

	//On vérifie si il faut commencer l'afficher une ligne plus bas 
	if(cursor->row+1-cursor->first_line > maxr-2) cursor->first_line++;


	// La ligne courante voit sont nombre maximum de colonne changer en la position où a lieu le saut
	max_cursor->col[cursor->row-1]=cursor->col;
	insertChar(cursor->row,cursor->col,text,'\n',max_cursor);
	
	cursorRowPlus(cursor,max_cursor);
	/* si le saut n'a pas lieu au milieu du texte il faut modifier le tableau max_cursor.col en décalant les cases 
	vers la droite et en ajoutant à la nouvelle ligne sont max */
	max_cursor->row++;	
	if((max_cursor->col=(int*)realloc(max_cursor->col,max_cursor->row*sizeof(int)))==NULL)quit(EXIT_FAILURE,"Malloc a échoué");
	if(max_cursor->row>cursor->row+1 ){
		for(int i=max_cursor->row-1;i>=cursor->row;i--){

			
			
			max_cursor->col[i]=max_cursor->col[i-1];
		}

		max_cursor->col[cursor->row-1]=new_max_col;
	
	}else{
	
		max_cursor->col[cursor->row-1]=new_max_col;
	}
	

	cursor->col=1;	
	display_text(*text,*cursor,max_cursor,true);
	//On positionne le curseur au début de la nouvelle ligne 
	cursorXY(*cursor);
}
	    
	



/*Fonction permettant d'aérer handle input
		On insert un caractère dans le tableau de texte à la position courante 
*/
void insertInput(cursor_xy_t* cursor,cursor_max_t* max_cursor,text_t* text,char input,int maxr,int maxc){
	
	insertChar(cursor->row,cursor->col,text,input,max_cursor);
	if(cursor->col==maxc && cursor->row<maxr){
		cursorRowPlus(cursor, max_cursor);
		cursor->col=1;
	}else if(cursor->col<maxc)	
			cursorColPlus(cursor, max_cursor);
			
	display_text(*text,*cursor,max_cursor,true);
	cursorXY(*cursor);
	
}
	


/* Fonction permettant de gérer les différents mode : insertion et l'édition via une fonction externe . 
	On lit 3 caractère sur l'entrée standard, 
		si on a uniquement 1 caractère on vérifie si c'est 27 i.e 
			l'utilisateur à appuyer sur ESCAPE donc veut passer en mode édition , les autres commandes à un
				caractère ne pose pas de problème particulier mis à part leur traitement.
		si il y a plus de 3 caractère par exemple : je fais une boucle où vérifie chaque caractère dans 
			la variable stock puis j'effectue le traitement de la commande 
*/
void handleInput(cursor_xy_t* cursor,cursor_xy_t* mouse,cursor_max_t* max_cursor,text_t* text,int fd,struct termios old,struct termios new){
	char stock[3];
	/* key me permet de controler les séquences d'échappement , plus de détail en bas */
	int key[3];
	key[0]=0;
	key[1]=0;
	key[2]=0;

	/* Pour la recherche de mots */
	char word[64]; // En étant raisonnable les mots peuvent pas faire plus de 64 caractères
	int len_w=0;
	word_position_t position;


	//Les commandes du mode complexe seront stockés ici 
	char cmd[4];
	cmd[0]=':';
	cmd[3]='\0';
	
	int nb_r,retour_poll;
	int j=0;
	char input;

	/* Vitesse de la souris */
	mouse_speed_t speed_control;
	setSpeed(&speed_control,0.5);	
	
	/* Dimension du terminal */
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	int maxr= win.ws_row;	
	int maxc=win.ws_col;
	
	int fd_mouse=open("/dev/input/mice",O_RDONLY);
	if(fd_mouse<0 ){
		perror("open");
		exit(EXIT_FAILURE);
	}


	cursor_xy_t pos_max_3;

	/* Poll nous permet de gérer la souris et le mode edition/insertion en même temps */
	struct pollfd fds[2];
	
	fds[0].fd=fd_mouse;
	fds[1].fd=STDIN_FILENO;

	fds[0].events=POLLIN;
	fds[1].events=POLLIN;

	char* line_copy;
	
	/* Pour savoir s'il y a quelque chose à coller */
	bool paste=false;

	/* Permet de savoir si on recherche un  mot */
	bool search_w=false;

	/* Permet  de savoir si on est entrain de taper une commande "complexe" i.e ":" en édition */
	bool simple_cmd=false;

	/* Permet  de savoir si on est en mode édition ou non*/
	bool edition=false;
	int i;
	

	while(1){
				
		retour_poll=poll(fds,2,-1);
		if(retour_poll<0) pexit("poll");
		
		if(retour_poll){	
			
			if(fds[0].revents & POLLIN){
				/* gestion de souris */	
			
				handleMouseInput(cursor, max_cursor, mouse, fd_mouse,&speed_control,*text);
				fds[0].revents=0;
				
			}else if(fds[1].revents & POLLIN ){
				
				/* gestion du mode insertion et edition */
				display_cursor();
				fds[1].revents=0;	
				

				/* on lit par 3 caractère sur la sortie standard car la séquence d'échappement ayant le plus de caractère est 
						"\x1b[3~" qui correspond à la touche suppr*/
				nb_r=read(STDIN_FILENO,stock,3);
				i=0;
				while(nb_r>0){
					/* On fait un traitement caractère par caractère car les actions nécesssites soit 1 soit 2 soit 3 caractère */
						input=stock[i];
						i++;
						
						if(input==BACKSPACE ){
							if(simple_cmd){
								if(j==0){
									clear_line();
									cursorXY(*cursor);
									simple_cmd=false;
								}else {
									cursor_xy_t pos_max;
									pos_max.row=maxr,
									pos_max.col=1;
									pos_max.first_line=0;

									cmd[j]='\0';
									j--;
									cursorXY(pos_max);
									clear_line()
									write(0,cmd,j+1);
								}

							
							}else if(search_w){
								if(len_w==0){
									clear_line();
									cursorXY(*cursor);
									
									search_w=false;

								}else {
									cursor_xy_t pos_max;
									pos_max.row=maxr,
									pos_max.col=1;
									pos_max.first_line=0;

									cmd[len_w]='\0';
									len_w--;
									cursorXY(pos_max);
									clear_line();
									write(0,"/",1);
									write(0,word,len_w);
								}



							}else if(edition && !simple_cmd && !search_w){
 								
 								mvLeft(cursor,max_cursor);

 							}else if(cursor->col>1 || ((cursor->col==0||cursor->col==1) && cursor->row>1)){
	 							
	 							backspaceChar(cursor,max_cursor,text);
								display_text(*text,*cursor,max_cursor,!edition);
								cursorXY(*cursor);
							}
						}else if(input==LINEFEED){
							
							if(simple_cmd){
							/* Si on entre une commande complex , "Entrer" nous permet de confirmer la commande */
								j++;
								cmd[j]=0;

								switch(cmd[1]){
									case 'q': 
										switch(cmd[2]){
											case 0: 
												if(!line_copy) free(line_copy);
												close(fd);
												freeText(text);
												return;
												break;
											case 'w': 
												if(!line_copy) free(line_copy);
												close(fd);
												saveText(*text);
												freeText(text);
												return;


											default:
												errorCmd(cursor,maxr);
												cmd[0]=':';
												cmd[1]=0;
												cmd[2]=0;
												j=0;
												simple_cmd=false;
										}
										break;
									case 'v':  

												
											 	if(modifSpeedCmd(cmd[2],&speed_control)==0)errorCmd(cursor,maxr);
											 	else successCmd(cursor,maxr);
											 	cmd[0]=':';
												cmd[1]=0;
												cmd[2]=0;
												j=0;
												simple_cmd=false;

											break;

									case 'w':
										switch(cmd[2]){
											case 0: 
												close(fd);
												saveText(*text);
												break;
											default:
												errorCmd(cursor,maxr);
												cmd[0]=':';
												cmd[1]=0;
												cmd[2]=0;
												j=0;
												simple_cmd=false;
										}
										break;
									default:
										errorCmd( cursor,maxr);
										cmd[0]=':';
										cmd[1]=0;
										cmd[2]=0;
										j=0;
										simple_cmd=false;
										break;
									}
								
								}else if (search_w){
									
									word[len_w]=0;
									shift_table_t shift;
									createShiftTab(&shift,word);
									searchWord(&position,word,*text,shift);
		
									/* Recherche et colors les mots trouvés */
									displayWord(word ,*cursor,*text,position);
									cursorXY(*cursor);
									/* Permet de se déplacer vers l'avant et l'arrière avant j et l , q nous permet de quitter la boucle */
									mvWord(position,cursor,word,*text);
									/* Réaffiche le text sans couleur */

									display_text(*text,*cursor,max_cursor,!edition);
									cursorXY(*cursor);
									search_w=false;
									free(position.pos);
									free(shift);
									position.pos=NULL;
									shift=NULL;

								}else if(edition){
									/* En mode edition on passe une juste à la ligne du dessus
										et au premier caractère  | comme dans vim "entrer" devient une touche pour déplacer
										le curseur */
									cursor->col=1;
									cursorRowPlus(cursor,max_cursor);
									cursorXY(*cursor);
								}else if(!edition){
									/* Mode insertion touche "entrer" classique , ce qui à après le curseur devient une nouvelle ligne 
										en dessous de celle courante */
									lineFeed(cursor,max_cursor,text,maxr);
								}
		
						}else if(key[0]==1 && key[1]==1 && key[2]==1){
							/* Trois caractère d'une séquence d'échappement on été lu */
							
							if(input==DELETE){
								/* Si c'est la touche de suppression on gère ce cas */
									delChar(cursor, max_cursor,text );

									display_text(*text,*cursor,max_cursor,!edition);
									cursorXY(*cursor);
								}else if(!edition){
									/* On insert le caractère  dans les autres cas */
										insertInput(cursor,max_cursor,text,input,maxr,maxc);

								}

							key[0]=0;
							key[1]=0;
							key[2]=0;

						}else if(key[0]==1 && key[1]==1 ){
							/* On a une séquence d'échappement ici "\x1b[" a été lu */
							switch(input){

								case 51 :
									//suppresion de caractère possible 
									key[2]=1;
									break;
							
								case ENDLINE:
									/* Place le curseur au dernier caractère de la ligne */
									cursor->col=max_cursor->col[cursor->row-1];
									cursorXY(*cursor);
									displayPos(*cursor);
									break;
								
								case LINE_BEGINING :
									/* Place le curseur au premier caractère de la ligne */
									cursor->col=1;
									cursorXY(*cursor);
									displayPos(*cursor);
									break;
								/* Les 4 cas suivant sont la gestions des flèches */
								case UP : 
									mvUp(cursor,max_cursor,text);	
									break; 
						
								case DOWN :
									mvDown(cursor,max_cursor,maxr,text);
									break;
						
								case RIGHT :	
									mvRight(cursor,max_cursor);
									break;
					
								case LEFT : 
									if(cursor->col>1)mvLeft(cursor,max_cursor);
									break;
							}
							if(input!=51){

								key[0]=0;
								key[1]=0;
							}
				
						}else if(input==27){
							/* Gestion du caractère \x1b==27 */
							if(nb_r==1){
								/* Seul ce caractère correspond à une pression sur la touche échape */

								if(simple_cmd || search_w){

									/* Lors d'une commande complexe 
											on efface la ligne de commande et on revient à la position précédente */
									write(0,"\x1b[2K",4	);												
									
									if(simple_cmd)simple_cmd=false;
									if(search_w)search_w=false;
									
									cursorXY(*cursor);

								}else if(!edition){
									/* Entre en mode édition */
									edition=true;
									
									setenv("MODE","edition",1);
								
									cursor_xy_t pos_max;
									pos_max.row=maxr,
									pos_max.col=1;
									pos_max.first_line=0;
									
									cursorXY(pos_max);
									write(0,"\x1b[2K",4	);
									cursorXY(*cursor);
								}												

							}else{
								
								key[0]=1;
							}
					
						}else if(input==91 && key[0]==1 && key[1]==0){
						
							key[1]=1;

						}else if(!edition){	

							/* Tous les cas gérer par l'éditeur sont vérifiés on peut insérer le caractère */
					 		insertInput(cursor,max_cursor,text,input,maxr,maxc);
					
						}else if(edition) {


							/* Gestion du mode edition */

							if(search_w){
								if(input!='\n'){
									word[len_w]=input;
									len_w++;
								}
								write(0,&input,1);

							}else if(simple_cmd){
								if(j==2){
									color(RED);
									write(0, "   Attention : Les commandes sont uniquement q,w et ..",53);
									sleep(1);
									write(0,"\x1b[2K",4	);
							
									cmd[0]=':';
									cmd[1]='\0';
									cmd[2]=0;

									j=0;
									color(WHITE);
	
									cursor_xy_t pos_max_2;
	
									pos_max_2.row=maxr,
									pos_max_2.col=2;
									pos_max_2.first_line=0;
								
									cursorXY(pos_max_2);
									write(0,cmd,2);												
							}else{
								j++;
								cmd[j]=input;
								write(0,&input,1);
							}

						}else {
							int len_line; 
							int coef;

							switch(input){
								case 'H': 
										cursor->row=cursor->first_line+1;
										cursor->col=1;
										cursorXY(*cursor);
										break;
								case 'G':
										coef= max_cursor->row/maxr;
										cursor->row=max_cursor->row;
										cursor->first_line= (coef*maxr);
										cursor->col=1;
										display_text(*text, *cursor, max_cursor,!edition);
										cursorXY(*cursor);
										break;
								


								case 'w':

										nextWord(cursor,*text);
										break;
								case 'b':


										prevWord( cursor,*text);

										break;
                                case 'M':

                                		coef= cursor->row/maxr;
										if(max_cursor->row<(maxr*(coef+1))-1){

											cursor->row=(cursor->first_line+max_cursor->row-1)/2;
											cursor->col=1;
											cursorXY(*cursor);

										}else{
											
											cursor->row= (cursor->first_line+maxr-1)/2;

											cursor->col=1;
											display_text(*text, *cursor, max_cursor,!edition);
											cursorXY(*cursor);
										}
                                		break;


								case 'L':
										coef= cursor->row/maxr;
										if(max_cursor->row<(maxr*(coef+1))-1){
											cursor->row=max_cursor->row-1+1;
											cursor->col=1;
											cursorXY(*cursor);

										}else{
											
											cursor->row= cursor->first_line+maxr-1;
											cursor->col=1;
											display_text(*text, *cursor, max_cursor,!edition);
											cursorXY(*cursor);
										}
											break;
									
								case 'g':
										cursor->row=1;
										cursor->col=1;
										cursor->first_line=0;
										display_text(*text, *cursor, max_cursor,!edition);
										cursorXY(*cursor);
										break;											

								case '$':
									cursor->col=max_cursor->col[cursor->row-1];
									cursorXY(*cursor);
									displayPos(*cursor);
									break;
								case 'O':
									cursor->col=1;
									cursorXY(*cursor);
									displayPos(*cursor);
									break;
								
								
								case 'v' :	
									
									len_line= max_cursor->col[cursor->row-1]-cursor->col+1;	
									if((line_copy=(char*)malloc(len_line+1*sizeof(char)))==NULL)quit(EXIT_FAILURE,"malloc a échoué");
									copyLine(*cursor , *max_cursor,*text,&line_copy);
									

									paste=true;

									break;
								case 'i':
								/* On revient au mode insertion */
									edition=false;
								 	simple_cmd=false;
								 	break;
								
								case 'x' :
								/* Suppression du caractère courant */
									delChar(cursor, max_cursor,text );
									display_text(*text,*cursor,max_cursor,!edition);
									cursorXY(*cursor);
									break;
								case ':' :
									/*Active l'interpreteur de commande complexe */
								    
									pos_max_3.row=maxr;
									pos_max_3.col=1;
									pos_max_3.first_line=0;
									cursorXY(pos_max_3);
									write(0,&input,1);
								
									simple_cmd=true;
									
							
									break;
								case '/': 
								/* Recherche de mot */

									pos_max_3.row=maxr;
									pos_max_3.col=1;
									pos_max_3.first_line=0;
									cursorXY(pos_max_3);
									write(0,&input,1);
									len_w=0;
									word[0]=0;
									
									search_w=true;

									break;
								}


							}
						
					
					}
					nb_r--;

				}
			}
		}
	}
}














/* Fonction d'affichage d'erreur lors du mode edition + commande */

void errorCmd(cursor_xy_t* cursor,int maxr){
	
	color(RED);
	write(0,"\x1b[2K",4	);
	cursor_xy_t pos_max;
	pos_max.row=maxr,
	pos_max.col=1;
	pos_max.first_line=0;
							
	cursorXY(pos_max);
	write(0, "COMMANDE INCONNUE",18);
	sleep(1);
	write(0,"\x1b[2K",4	);
	color(WHITE);
	cursorXY(*cursor);
}


void successCmd(cursor_xy_t* cursor,int maxr){
	
	color(GREEN);
	write(0,"\x1b[2K",4	);
	cursor_xy_t pos_max;
	pos_max.row=maxr,
	pos_max.col=1;
	pos_max.first_line=0;
							
	cursorXY(pos_max);
	write(0, "Effecuté!",18);
	sleep(1);
	write(0,"\x1b[2K",4	);
	color(WHITE);
	cursorXY(*cursor);
}









void saveText(text_t t){
	//on réccuppère les dimensions de notre fenêtre

	int temp_int=0;
	node_char_t* node;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	int maxc= win.ws_col;
	char car[1];


	const char* path=getenv("_PATHFILE");
	int fd=open(path,O_WRONLY|O_TRUNC);
	if(t.line_stk==NULL)return;

	bool next_list=true;
	int col =0;
	while( temp_int < t.nb_row ){
		if(next_list) node=(t.line_stk)[temp_int];

		while(col<maxc && node!=NULL){
			car[0]= node->value;
			if(car[0]!='\n'){				
				if(car[0]=='\r')car[0]='\n';
				write(fd,car,1);
			}
			
			
			
			col++;
			node=(node_char_t*)node->next;

		}	
		if(col==maxc && node!=NULL) {

			next_list=false;
			
		}else if( node==NULL ){
			col=0;
			next_list=true;
			temp_int++;
		}
		
	}
		
	close(fd);



}



void freeLine(node_char_t** l){
	 if((*l)==NULL)return ; 
	 freeLine((node_char_t**)&((*l)->next));
	 free(*l);
	
}

//libère la mémoire de notre structure de text
void freeText(text_t* del){
	int i=0;
	while(i<del->nb_row){
		
		if(del->line_stk[i])freeLine(&(del->line_stk)[i]);
		i++;
	}
	free(del->line_stk);
}
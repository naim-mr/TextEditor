#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "window.h"




static text_t text;



/*

static void sigwinch_hand(int signum){
	struct termios temp;
	tcgetattr(0,&temp);
	tcsetattr(0,TCSAFLUSH,&old);

	display_text(text,cursor,&max_cursor,false);
	write(0,"\x1b[H",3);

	tcsetattr(0,TCSAFLUSH,&temp);
	
}
*/

int main(int argc,char** argv){


	
	clear_screen();
	// Initialisation de mes structures de gestion du curseur : voir window.h 
	max_cursor.row=1;
	if((max_cursor.col=(int*)malloc(sizeof(int)))==NULL)quit(EXIT_FAILURE,"Echec malloc ");
	max_cursor.col[0]=1;
   	cursor.row=1;
   	cursor.col=1;
   	cursor.nb=1;
   	cursor.first_line=0;

   	mouse.nb=2;
   	mouse.row=1;
   	mouse.col=1;

   	char input;
	

   	// Initialisation de  la structure stockant le texte  /
  	text= newText();

  	/*
	//On gère le signalement d'un changement de taille du terminal
	struct sigaction terminal_resize; 
    terminal_resize.sa_handler= sigwinch_hand;
   	terminal_resize.sa_flags=SA_RESTART;
  	sigaction(SIGWINCH,&terminal_resize,NULL);*/


  	
 	// on stock les anciens "paramètre" du terminal 
	tcgetattr(0,&old);
	tcgetattr(0,&new);

	
	int fd,nb_r;
  	char buf[BUFSIZE];
	

	if(argc==2){
		 fd=open(argv[1],O_RDWR|O_CREAT,0644);
		 setenv("_PATHFILE", argv[1], 0);
  	}
  	if(argc>2){
  		write(0,"Vous pouvez passer qu'un seul argument",37);
  		exit(1);
  	}else if(argc==1){
		write(0,"Veuillez donner un fichier en argument",37);
  		exit(1);
	}

	if(fd<0){
		//l'ouverture a échoué
		quit(EXIT_FAILURE,"open");
	}else { 
	    if((nb_r=read(fd,buf,BUFSIZE))>-1){
	    	
	    	if(nb_r==0){
			    buf[0]='~';
			   	buf[1]=0;  		
	    	}else {
	    		buf[nb_r]=0;
	    	}

			// On parse le texte par ligne dans la structure
			parseText(buf,&text,nb_r,&max_cursor);
			
			display_text(text,cursor,&max_cursor,true);
			//Fonction déplçant le curseur
			cursorXY(cursor);
				
		}	
	}

	// Passage en mode non canonique de l'entrée standard avec quelque option 
	
	new.c_lflag &= ~( ECHO| ICANON );	
	new.c_iflag &= ~( IXON);
	new.c_cflag |= CS8;
	new.c_oflag &= ~OPOST;

	tcsetattr(0,TCSAFLUSH,&new);
	

	
	write(1,"\033[m",3);
	//Fonction principal pour l'éditeur qui traite la lecture et l'écriture sur l'entrée standard

	handleInput(&cursor,&mouse,&max_cursor,&text,fd,old,new);
	

	//remise à l'état initial de la sortie standard
	tcsetattr(0,TCSAFLUSH,&old);
	free(max_cursor.col);
	max_cursor.col=NULL;
	unsetenv("_PATHFILE");
	close(fd);
	clear_screen()
	
	return EXIT_SUCCESS;
}

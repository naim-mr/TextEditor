EXEC=editeur
SOURCES=editeur.c window.c mouse.c search.c edit.c
OBJECTS=$(SOURCES:.c=.o)
CC=gcc 
CFLAGS= -W -Wall  -std=gnu99 -g 

 
.PHONY: default clean
 
default: $(EXEC)

editeur.o: editeur.c window.h mouse.h
window.o: window.c mouse.h search.h edit.h
mouse.o: mouse.h window.h
search.o: search.h window.h
edit.o: edit.c window.h
 
%.o: %.c
		$(CC) -o $@ -c   $< $(CFLAGS)

$(EXEC): $(OBJECTS)
		$(CC)  -o $@ $^

clean:
	rm -rf $(EXEC) $(OBJECTS) $(SOURCES:.c=.c~) $(SOURCES:.c=.h~) Makefile~
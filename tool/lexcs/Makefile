##############################################
# LEXCS Makefile
##############################################

AR = ar
CC = gcc
CFLAGS = -O2  
#CFLAGS = -O2 -DDEBUG -g 
INCLUDE = -I. 


PROGRAMS = lexcs 
SOURCES = lexcs.c 


OBJECTS = $(SOURCES:.c=.o)

all: $(PROGRAMS) 

$(PROGRAMS) : $(OBJECTS)
	$(CC) $(INCLUDE) $(CFLAGS) -o $@ $< 

clean:
	@rm -f *.o $(PROGRAMS)
	


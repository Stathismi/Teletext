CFLAGS = -O2 -Wall -Wextra -pedantic -ansi -Wfloat-equal -Werror

LIBS =  `pkg-config sdl2 --libs`
CC = gcc

all: teletext 

teletext : teletext
	$(CC) teletext.c -o teletext -O3 $(CFLAGS) $(LIBS)

run: all 
	./teletext test.m7 m7fixed.fnt
	./teletext panda.m7 m7fixed.fnt
	./teletext lfc.m7 m7fixed.fnt

clean:
	rm -f teletext

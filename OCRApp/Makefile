EXEC_NAME = OCR.out
CFLAGS = -W -Wall -Wextra  -ansi -pedantic -std=c99 `pkg-config --cflags gtk+-3.0`
LIBRARIES = -l SDL -l SDL_image -lm -ldl `pkg-config --libs gtk+-3.0`

SRC=sources/*.c

all:main tmp

main:$(SRC) main.c
	$(CC) $(CFLAGS) $(LIBRARIES) $(SRC) main.c -o $(EXEC_NAME)

tmp:
	mkdir tmp

clean:
	${RM} *.o          # remove object files
	${RM} $(EXEC_NAME) # remove main program
	rm -rf tmp

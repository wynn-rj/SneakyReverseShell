CC=gcc
OUTPUT=server.o
SOURCE=server.c
DEBUGFLAGS = -g -Wall -DDEBUG

all:
	$(CC) -o $(OUTPUT) $(SOURCE)

debug:
	$(CC) -o $(OUTPUT) $(SOURCE) $(DEBUGFLAGS)

clean:
	rm $(OUTPUT)

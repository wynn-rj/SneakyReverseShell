CC=gcc
OUTPUT=server.o
SOURCE=server.c
INCLUDE=-lpthread
DEBUGFLAGS = -g -Wall -DDEBUG

all:
	$(CC) -o $(OUTPUT) $(SOURCE) $(INCLUDE)

debug:
	$(CC) -o $(OUTPUT) $(SOURCE) $(INCLUDE) $(DEBUGFLAGS)

clean:
	rm $(OUTPUT)

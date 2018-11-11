CC=gcc
OUTPUT=client.o
SOURCE=client.c
INCLUDE=-lpthread
DEBUGFLAGS= -g -Wall -DDEBUG

all:
	$(CC) -o $(OUTPUT) $(SOURCE) $(INCLUDE)

debug:
	$(CC) -o $(OUTPUT) $(SOURCE) $(INCLUDE) $(DEBUGFLAGS)

clean:
	rm $(OUTPUT)

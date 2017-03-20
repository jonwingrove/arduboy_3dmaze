CC 		= gcc
SRCPATH		= src/
OBJPATH		= obj/
BINPATH		= bin/
CFLAGS 		= -Wall -g
LDFLAGS 	= $(CFLAGS) -static
ALLOBJS		= $(OBJPATH)main.o

all: linux

linux: $(BINPATH)tunegen

$(BINPATH)tunegen: $(ALLOBJS)
	$(CC) $(LDFLAGS) -o $(BINPATH)tunegen $(ALLOBJS)


$(OBJPATH)main.o: $(SRCPATH)main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJPATH)*.o
	rm -f $(BINPATH)tunegen

.PHONY: clean

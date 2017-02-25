CC 		= gcc
SRCPATH		= src/
OBJPATH		= obj/
BINPATH		= bin/
CFLAGS 		= -Wall -g
LDFLAGS 	= $(CFLAGS) -static
ALLOBJS		= $(OBJPATH)main.o

all: linux

linux: $(BINPATH)maze

$(BINPATH)maze: $(ALLOBJS)
	$(CC) $(LDFLAGS) -o $(BINPATH)maze $(ALLOBJS)


$(OBJPATH)main.o: $(SRCPATH)main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJPATH)*.o
	rm -f $(BINPATH)maze

.PHONY: clean

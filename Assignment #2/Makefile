OBJECTS = cshell

all: $(OBJECTS)

cshell: cshell.c cshell.h
	gcc -g -Wall -o cshell cshell.c

.PHONY: clean

clean:
	rm $(OBJECTS)
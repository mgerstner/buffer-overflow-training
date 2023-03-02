MAIN = gdbtest
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
CFLAGS ?= -g

$(MAIN): $(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) -o $(MAIN)

%.o: %.c
	gcc $(CFLAGS) -c -Wall -Werror $< -o $@

clean:
	rm -f $(MAIN) $(OBJECTS)

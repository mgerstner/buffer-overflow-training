MAIN = dump_head
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
FLAGS = -fno-omit-frame-pointer -O0 -g -Wall

$(MAIN): $(OBJECTS)
	gcc $(FLAGS) $(OBJECTS) -o $(MAIN)

%.o: %.c
	gcc $(FLAGS) -c $< -o $@

clean:
	rm -f $(MAIN) $(OBJECTS)

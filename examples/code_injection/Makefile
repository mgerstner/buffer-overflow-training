MAIN = kitty2
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
FLAGS = -fno-omit-frame-pointer -fno-stack-protector -g -O0 -Wall -Wno-deprecated-declarations -Werror

$(MAIN): $(OBJECTS)
	gcc $(FLAGS) $(OBJECTS) -o $(MAIN)
	execstack -s $(MAIN) || rm $(MAIN)

%.o: %.c
	gcc $(FLAGS) -c  $< -o $@

clean:
	rm -f $(MAIN) $(OBJECTS)

include ../common/Makefile

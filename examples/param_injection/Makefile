MAIN = run_safe_prog
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
FLAGS = -fno-omit-frame-pointer -fno-stack-protector -O0 -g -Wall -Werror

$(MAIN): $(OBJECTS)
	gcc $(FLAGS) $(OBJECTS) -o $(MAIN)

%.o: %.c
	gcc -c $(FLAGS) $< -o $@

clean:
	rm -f $(MAIN) $(OBJECTS)

include ../common/Makefile

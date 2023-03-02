MAIN = kitty
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
# NOTE: without -no-pie, objdump will not deliver us the runtime address,
# because the code is relocated. Therefore to simplify the demo we add this
# flag here.
FLAGS = -fno-omit-frame-pointer -fno-stack-protector -O0 -g -Wall -no-pie

$(MAIN): $(OBJECTS)
	gcc $(FLAGS) $(OBJECTS) -o $(MAIN)

%.o: %.c
	gcc -c $(FLAGS) $< -o $@

clean:
	rm -f $(MAIN) $(OBJECTS)

include ../common/Makefile

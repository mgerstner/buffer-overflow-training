PRELOAD = geteuid-preload/libgeteuid_preload.so
SOURCES = $(wildcard geteuid-preload/*.c)
OBJECTS = $(SOURCES:.c=.o)
FLAGS = -shared

$(PRELOAD): $(OBJECTS)
	gcc $(FLAGS) $(OBJECTS) -o $(PRELOAD)

%.o: %.c
	gcc -c $(FLAGS) $< -o $@ -DREAL_USER_ID=$(shell id -u)

clean:
	rm -f $(PRELOAD) $(OBJECTS)

shell:
	setarch `uname -m` -R /bin/bash

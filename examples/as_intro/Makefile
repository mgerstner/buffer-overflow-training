MAIN = intro
SOURCES = $(wildcard *.s)
OBJECTS = $(SOURCES:.s=.o)

$(MAIN): $(OBJECTS)
	ld $(OBJECTS) -o $(MAIN)

%.o: %.s
	as -g $< -o $@

clean:
	rm -f $(MAIN) $(OBJECTS)

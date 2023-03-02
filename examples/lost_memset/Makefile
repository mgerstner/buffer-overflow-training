MAIN = lost_memset
FLAGS = -g -O2

all: $(MAIN)

$(MAIN): lost_memset.c
	gcc $(FLAGS) $< -o $@

$(MAIN).nobuiltin: lost_memset.c
	gcc $(FLAGS) -fno-builtin-memset $< -o $@

clean:
	rm -f $(MAIN) $(MAIN).nobuiltin

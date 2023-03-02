BINS=exit_asm exit_bin
all: $(BINS)

exit_asm: exit_asm.c
	gcc -g $< -o $@

exit_bin: exit_bin.c
	@which execstack >/dev/null 2>&1 || ( echo "you need to install 'execstack'" && exit 1 )
	gcc -g $< -o $@
	# this is necessary to avoid a SEGFAULT because of writing to the text
	# segment
	execstack -s $@ || rm $@

clean:
	rm -f *.o $(BINS)

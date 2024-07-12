BINS=exec_asm exec_bin
EXTRA_BINS=exec_asm_mprotect
all: $(BINS)

exec_asm: exec_asm.c
	gcc -g $< -o $@

exec_bin: exec_bin.c
	gcc -g $< -o $@
	execstack -s $@ || rm $@

clean:
	rm -f *.o $(BINS) $(EXTRA_BINS)

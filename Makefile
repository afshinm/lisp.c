CFLAGS=-std=c17 -g -O2 -Wall

.PHONY: clean test

lispc: lispc.c

clean:
	rm -f lispc *~

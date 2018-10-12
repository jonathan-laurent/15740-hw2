CC=gcc
CFLAGS = -std=gnu11
LFLAGS = -lrt -lpthread

HANDINFILES = writeup.pdf Makefile mountain.c cores.c linesize.c smt.c lock.c mmt.c

all: mmt lock smt mountain cores linesize

submit:
	tar cvf submission.tar.gz $(HANDINFILES)

full: all mountain.png
	./linesize > linesize.txt
	./cores > cores.txt

linesize: linesize.c
	$(CC) $(CFLAGS) $(LFLAGS) $^ -o $@

cores: cores.c
	$(CC) $(CFLAGS) $(LFLAGS) $^ -o $@

mountain: mountain.c

mountain.png: mountain plot.py
	./mountain simple > mountain.data
	./plot.py mountain.data --sections -o mountain.png

test: mountain.png
	open mountain.png

mmt: func_time.c perf.c mmt.c atomic.S
	$(CC) $(CFLAGS) $(LFLAGS) $^ -o $@

lock: lock.c atomic.S func_time.c perf.c
	$(CC) $(CFLAGS) $(LFLAGS) $^ -o $@

smt: smt.c func_time.c perf.c
	$(CC) $(CFLAGS) $(LFLAGS) -D_GNU_SOURCE $^ -o $@

clean:
	rm -rf mountain mountain.png *~ mountain.data
	rm -rf linesize linesize.txt cores cores.txt
	rm -rf mmt
	rm -rf lock
	rm -rf smt

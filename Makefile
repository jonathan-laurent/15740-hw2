CC=gcc
CFLAGS = -std=gnu11
LFLAGS = -lrt -lpthread

all: mountain cores linesize

full: all mountain.png
	./linesize > linesize.txt
	./cores > cores.txt

linesize: linesize.c

cores: cores.c

mountain: mountain.c

mountain.png: mountain plot.py
	./mountain simple > mountain.data
	./plot.py mountain.data --sections -o mountain.png

test: mountain.png
	open mountain.png

atomic.o: atomic.S
	$(AS) $(ASFLAGS) -c $< -o $@ $(LDFLAGS)

matmul.o: matmul.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

matmul: matmul.o atomic.o
	$(CC) $(CFLAGS) func_time.c perf.c matmul.o atomic.o -o $@ $(LFLAGS)

lock.o: lock.c
	$(CC) $(CFLAGS) -c $< -o $@

lock: lock.o atomic.o
	$(CC) atomic.S lock.c func_time.c perf.c $(CFLAGS) $(LFLAGS) -o lock


clean:
	rm -rf mountain mountain.png *~ mountain.data
	rm -rf linesize linesize.txt cores cores.txt
	rm -rf atomic.o matmul.o matmul
	rm -rf lock.o lock

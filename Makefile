all: mountain

mountain: mountain.c

mountain.png: mountain
	./mountain simple > mountain.data
	./plot.py mountain.data -o mountain.png

test: mountain.png
	open mountain.png

clean:
	rm -rf mountain mountain.png *~ mountain.data

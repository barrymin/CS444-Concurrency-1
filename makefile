default: all
all:
	gcc -g -o prodCon prodCon.c -lpthread

clean:
	rm prodCon


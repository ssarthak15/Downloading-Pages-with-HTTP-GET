all: sclient sserver

sclient: sclient.c
	gcc -o sclient sclient.c

sserver: sserver.c
	gcc -o sserver sserver.c

clean:
	rm sserver sclient

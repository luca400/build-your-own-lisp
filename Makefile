build: parsing.c
	gcc -Wall -pedantic parsing.c mpc.c  -ledit -lm -o parsing
clean:
	rm -f parsing

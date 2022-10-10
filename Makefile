build: parsing.c
	gcc parsing.c mpc.c  -ledit -lm -o parsing
clean:
	rm -f parsing

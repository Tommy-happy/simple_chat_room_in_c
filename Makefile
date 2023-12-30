server-client: server.c client.c
	gcc server.c -g -fopenmp -o server
	gcc client.c -lncurses -fopenmp -o client

ncurses:
	gcc test_ncurses.c -lncurses -o test_ncurses

install:
	sudo apt-get install libncurses5-dev
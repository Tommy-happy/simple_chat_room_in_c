server-client: server.c client.c
	gcc server.c -fopenmp -o server
	gcc client.c -fopenmp -o client
	# gcc client2.c -fopenmp -o client2

ncurses:
	gcc test_ncurses.c -lncurses -o test_ncurses

install:
	sudo apt-get install libncurses5-dev
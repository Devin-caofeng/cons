all:multi_thread_serv cli

multi_thread_serv:../wrap.c multi_thread_serv.c
	gcc -o $@ $^ -Wall -g -I../ -pthread

cli:../wrap.c cli.c
	gcc -o $@ $^ -Wall -g -I../

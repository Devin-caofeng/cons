all:multi_proc_serv cli

multi_proc_serv:../wrap.c multi_proc_serv.c
	gcc -o $@ $^ -Wall -g -I../

cli:../wrap.c cli.c
	gcc -o $@ $^ -Wall -g -I../

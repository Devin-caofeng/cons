all:multi_proc_serv multi_proc_cli

multi_proc_serv:../wrap.c multi_proc_serv.c
	gcc -o $@ $^ -Wall -g -I../

multi_proc_cli:../wrap.c multi_proc_cli.c
	gcc -o $@ $^ -Wall -g -I../

all:serv cli

serv:select_server.c ../wrap.c
	gcc -std=c99 -o $@ $^ -I../ -Wall -g

cli:cli.c ../wrap.c
	gcc -o $@ $^ -I../ -Wall -g

.PHONY:
clean:
	-rm -rf serv cli

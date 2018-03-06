all:serv cli

serv:epoll_serv.c ../wrap.c
	gcc -std=gnu99 -o $@ $^ -I../ -Wall -g

cli:cli.c ../wrap.c
	gcc -std=gnu99 -o $@ $^ -I../ -Wall -g


.PHONY:
clean:
	-rm -rf serv cli

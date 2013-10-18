all: smashme-prog configure-prog

smashme-prog: smashme.c smashme.h
	gcc -g -o smashme smashme.c

configure-prog: configure.c smashme.h
	gcc -o configure configure.c

addresses:
	nm smashme | egrep '(MyStack|Uncalled)'

clean:
	rm -rf smashme configure *.dSYM

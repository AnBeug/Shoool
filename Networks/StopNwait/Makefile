CC = gcc
CFLAGS = -g -Wall
#LIBS = -lm -lpthread
SENDERR = sendErrH
DIR =
SUNDIR = sun/

OS = $(shell uname -s)
ifeq ("$(OS)", "SunOS")
	LIBS += -lsocket -lnsl
	DIR = $(SUNDIR)
endif

all: $(DIR) $(DIR)rput $(DIR)server

$(DIR):
	mkdir $(DIR)

$(DIR)rput: rput.c socket_utils.c sendtoErrG.c networks.h
	$(CC) $(CFLAGS) -o $(DIR)rput rput.c socket_utils.c sendtoErrG.c checksum.c $(LIBS)

$(DIR)server: server.c socket_utils.c sendtoErrG.c networks.h
	$(CC) $(CFLAGS) -o $(DIR)server server.c socket_utils.c sendtoErrG.c checksum.c $(LIBS)

clean:
	rm -rf rput server *.dSYM

##############################################
# File          : Makefile                   #
# Author        : Annie Beug                 #
# Date          : 28-FEB-2010                #
# Modified      : 28-FEB-2010                #
# Project       : Assignment 4               #
# Class         : CPE 464                    #
# Professor     : Dr. Smith                  #
##############################################

CC = gcc
CFLAGS = -g -Wall
#LIBS = -lm -lpthread
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

$(DIR)rput: rput.c socket_utils.c sendtoErrG.c networks.h debugger.h
	$(CC) $(CFLAGS) -o $(DIR)rput rput.c socket_utils.c sendtoErrG.c checksum.c $(LIBS)

$(DIR)server: server.c socket_utils.c sendtoErrG.c networks.h debugger.h
	$(CC) $(CFLAGS) -o $(DIR)server server.c socket_utils.c sendtoErrG.c checksum.c $(LIBS)

# clean targets for Solaris and Linux
clean:
	-rm rput server


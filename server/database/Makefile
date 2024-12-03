CC = g++

CFLAGS = -Wall
DBFLAGS = -l sqlite3

CLIENT_DIR = ./client
SERVER_DIR = ./server

IP_SERVER = 192.168.1.5


all: clean test.out run_test

test.out: 
	${CC} ${CFLAGS} test.cpp -o test.out ${DBFLAGS}

run_test: 
	./test.out

clean: 
	rm -f *.db *.out all

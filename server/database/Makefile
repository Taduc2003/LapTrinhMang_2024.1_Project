CC = gcc

CFLAGS = -Wall
DBFLAGS = -l sqlite3

all: clean test.out run_test

test.out: 
	${CC} ${CFLAGS} testDB.c -o testDB.out ${DBFLAGS}

run_test: 
	./testDB.out

clean: 
	rm -f *.db *.out all

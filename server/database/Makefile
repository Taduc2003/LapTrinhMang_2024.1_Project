CC = gcc
CFLAGS = -Wall
LDFLAGS = -l sqlite3

all: testDB.out

testDB.out: testDB.c
	$(CC) $(CFLAGS) -o testDB.out testDB.c $(LDFLAGS)

run_test: testDB.out
	./testDB.out

clean:
	rm -f testDB.out

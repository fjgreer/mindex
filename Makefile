#makefile for mindex

CC=gcc
CFLAGS=-c -Wall -Wextra -ggdb -std=c99 -march=native -pipe
LDFLAGS=-l sqlite3
SOURCES=db_funcs.c log_funcs.c
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(OBJECTS)

all-clean: clean test-clean

.c.o:
	$(CC) $(CFLAGS) $< -o $@

test: $(OBJECTS)
	$(CC) $(CFLAGS) db_test.c
	$(CC) $(LDFLAGS) $(OBJECTS) db_test.o -o dbt
	./dbt

test-clean:
	rm -rf ./dbt test.db *csv test-ppd.txt

clean:
	rm -rf *o

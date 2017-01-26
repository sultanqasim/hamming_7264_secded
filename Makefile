CC = gcc
CFLAGS = -O3 -Wall -Wextra -pedantic -fdiagnostics-color
LFLAGS =

HEADERS = $(wildcard *.h)

all: fec_test

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

fec_test: main.o fec_secded7264.o
	$(CC) $(LFLAGS) $^ -o $@

clean:
	rm -f *.o fec_test

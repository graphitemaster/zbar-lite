rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

CC ?= gcc
CFLAGS = -Iinclude -Izbar -Wall -Wextra -ffast-math -fPIC -O2
LDFLAGS = -lm -lpthread -L. -lzbar-lite
SRC = $(call rwildcard, , zbar/*.c)
OBJ = $(SRC:.c=.o)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

libzbar-lite.a: $(OBJ)
	ar rcs $@ $^

test: libzbar-lite.a example/test.o
	$(CC) example/test.o $(LDFLAGS) -o $@

clean:
	rm -f $(OBJ)
	rm -f test example/test.o

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

.PHONY: clean

proj2: proj2.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) -f proj2.zip *.o proj2

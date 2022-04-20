CC = gcc
CFLAGS = -std=gnu99 -Wextra -Werror -pedantic -g 
LDFLAGS = -pthread -g

.PHONY: clean

proj2: proj2.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) -f proj2.zip *.o proj2

SRC = test.c ftpin_cmd.c ftpin_server.c
OBJ = $(SRC:.c=.o)
TAR = test
CC = gcc
LDFLAGS = -lm -lpthread
CFLAGS = -c

$(TAR):$(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o $(TAR)

CC = gcc
CFLAGS = -Wall
LIB = -lrt -lpthread -lm
DEPS = memory.h helper.h
OBJ = main.o memory.o helper.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS) $(LIB)

sortArrays: $(OBJ)
	$(CC)  -o $@ $^ $(CFLAGS) $(LIB)

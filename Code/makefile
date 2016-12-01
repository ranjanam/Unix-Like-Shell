TARGET=shell
CC=g++
HEADERS=header.h
FLAGS=-Wall -g
OBJS=parser.o main.o lib.o builtIns.o

$(TARGET) : $(OBJS) $(HEADERS)
	$(CC) $(FLAGS) -o $@ $^
%.o : %.c 
	$(CC) $(FLAGS) -c -o $@ $^

clean : 
	rm *.o $(TARGET)
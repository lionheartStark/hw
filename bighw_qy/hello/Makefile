# app

CC = gcc
TARGET = server
OBJS = get_time.o http_session.o init_socket.o $(TARGET).o

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o : %.c
	$(CC) -c $^ -o $@

%.o : %.c
	$(CC) -c $^ -o $@

clean:
	rm -rf *.o
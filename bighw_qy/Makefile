# app

CC = gcc
# TARGET = server_iteration
TARGET = server_threadpool
# TARGET = server_processpool
# TARGET = server_prethread
OBJS = sembuf.o get_time.o http_session.o init_socket.o threadpool.o $(TARGET).o 

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o server -lpthread
	rm -rf *.o

%.o : %.c
	$(CC) -c $^ -o $@ 

%.o : %.c
	$(CC) -c $^ -o $@

clean:
	rm server

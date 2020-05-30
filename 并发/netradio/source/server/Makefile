
CFLAGS+=-I../include/ -pthread
LDFLAGS+=-pthread

all:server

server:server.o thread_channel.o thread_list.o mytbf.o medialib.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o server
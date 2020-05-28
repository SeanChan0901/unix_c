
CFLAGS+=-I../include/ -pthread
LDFLAGS+=-pthread

all:server

client:server.o
	gcc $^ -o $@ $(CFLAGS)

clean:
	rm -rf *.o server
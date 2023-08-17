CC = g++
CFLAGS = -Wall -Wextra
LDFLAGS = -lfltk -lcurl

all: proyecto

proyecto: src/main.o src/P2PMenu.o src/ConnectionManager.o
	$(CC) $(CFLAGS) -o proyecto src/main.o src/P2PMenu.o src/ConnectionManager.o $(LDFLAGS)

src/main.o: src/main.cpp src/P2PMenu.h
	$(CC) $(CFLAGS) -c src/main.cpp -o src/main.o

src/P2PMenu.o: src/P2PMenu.cpp src/P2PMenu.h
	$(CC) $(CFLAGS) -c src/P2PMenu.cpp -o src/P2PMenu.o

src/ConnectionManager.o: src/ConnectionManager.cpp src/ConnectionManager.h
	$(CC) $(CFLAGS) -c src/ConnectionManager.cpp -o src/ConnectionManager.o
clean:
	rm -f src/*.o proyecto
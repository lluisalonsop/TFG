CC = g++
CFLAGS = -Wall -Wextra -I/home/lluis/Documents/vcpkg/installed/x64-linux/include
LDFLAGS = -L/home/lluis/Documents/vcpkg/installed/x64-linux/lib -lfltk -lcurl -ljsoncpp

# Lista de archivos de código fuente
SRCS = src/main.cpp src/P2PMenu.cpp src/ConnectionManager.cpp src/RoundButton.cpp

# Objetos generados a partir de archivos fuente
OBJS = $(SRCS:.cpp=.o)

# Nombre del ejecutable
TARGET = proyecto

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
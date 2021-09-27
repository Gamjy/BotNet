SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUES = -L ../Bibliotheques -lipc -lreseau -lthread -pthread
EXECUTABLE = WebServer
CFLAGS += -Wall -Wextra -Werror -I ../Bibliotheques

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

clean:
	rm -rf $(EXECUTABLE) *.o

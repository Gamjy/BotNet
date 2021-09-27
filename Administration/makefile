SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUES = -L ../Bibliotheques -lipc -lthread -pthread
EXECUTABLE = admin
CFLAGS += -Wall -Wextra -Werror -I ../Bibliotheques

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

clean:
	rm -rf $(EXECUTABLE) *.o

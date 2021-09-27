SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUES = -L ../Bibliotheques -lreseau -lthread -pthread -ldl
EXECUTABLE = Boat
CFLAGS += -Wall -Wextra -rdynamic -I ../Bibliotheques 

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

clean:
	rm -rf $(EXECUTABLE) *.o



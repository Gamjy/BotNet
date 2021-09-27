SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUES = -L ../Bibliotheques -lreseau -lipc -lthread -pthread
EXECUTABLE = CandC
CFLAGS += -Wall -Wextra -I ../Bibliotheques 

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJETS)
	$(CC) -o $@ $^ $(BIBLIOTHEQUES)

clean:
	rm -rf $(EXECUTABLE) *.o

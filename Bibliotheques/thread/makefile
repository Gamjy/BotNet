SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUE = libthread.so


CFLAGS += -Wall -fpic

all: $(BIBLIOTHEQUE)

$(OBJETS): ../libthread.h

$(BIBLIOTHEQUE): $(OBJETS)
	$(CC) $^ -o $@ -shared
	mv $(BIBLIOTHEQUE) ../

clean:
	rm -rf *.o *.a *.so core

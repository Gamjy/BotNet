SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUE = libreseau.so


CFLAGS += -Wall -fpic

all: $(BIBLIOTHEQUE)

$(OBJETS): ../libreseau.h

$(BIBLIOTHEQUE): $(OBJETS)
	$(CC) $^ -o $@ -shared
	mv $(BIBLIOTHEQUE) ../

clean:
	rm -rf *.o *.a *.so core

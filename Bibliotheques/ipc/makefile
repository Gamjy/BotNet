SOURCES = $(wildcard *.c)
OBJETS = $(SOURCES:.c=.o)
BIBLIOTHEQUE = libipc.so


CFLAGS += -Wall -fpic

all: $(BIBLIOTHEQUE)

$(OBJETS): ../libipc.h

$(BIBLIOTHEQUE): $(OBJETS)
	$(CC) $^ -o $@ -shared
	mv $(BIBLIOTHEQUE) ../

clean:
	rm -rf *.o *.a *.so core

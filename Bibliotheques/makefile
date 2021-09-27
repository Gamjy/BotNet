REPERTOIRES = reseau ipc thread
COMPILE = $(REPERTOIRES:%=all-%)
NETTOYAGE = $(REPERTOIRES:%=clean-%)
export CFLAGS += -Wall -Wextra 

all: $(COMPILE)
$(COMPILE):
	$(MAKE) -C $(@:all-%=%)

clean: $(NETTOYAGE)
$(NETTOYAGE):
	$(MAKE) -C $(@:clean-%=%) clean


.PHONY: $(COMPILE) $(NETTOYAGE)
.PHONY: all clean

clean:
	rm -rf *.o *.a *.so core

REPERTOIRES = Bibliotheques Administration Boat CandC Web_server
COMPILE = $(REPERTOIRES:%=all-%)
NETTOYAGE = $(REPERTOIRES:%=clean-%)
export CFLAGS += -Wall -Wextra -Werror 

all: $(COMPILE)
$(COMPILE):
	$(MAKE) -C $(@:all-%=%)

clean: $(NETTOYAGE)
$(NETTOYAGE):
	$(MAKE) -C $(@:clean-%=%) clean


.PHONY: $(COMPILE) $(NETTOYAGE)
.PHONY: all clean

clean:
	rm -rf *.o *.a core

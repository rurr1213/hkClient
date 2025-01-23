
SUBDIRS := $(wildcard */.)
SUBDIRSCLEAN=$(addsuffix clean,$(SUBDIRS))

all: $(SUBDIRS)


clean: $(SUBDIRSCLEAN)


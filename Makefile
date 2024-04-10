# Makefile for Makefile.pdlibbuilder.


lib.name := phase_retrieval~

rtpghi~.class.sources = src/rtpghi~.c
rtisi~.class.sources = src/rtisi~.c

datafiles = rtpghi~-help.pd

ldlibs += -lltfat -lphaseret

PDLIBBUILDER_DIR=./pd-lib-builder
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

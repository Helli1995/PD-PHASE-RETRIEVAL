# Makefile for Makefile.pdlibbuilder.
#

lib.name := rtpghi

init_plan.class.sources = src/init_plan.c

ldlibs += -lltfat -lphaseret


PDLIBBUILDER_DIR=./pd-lib-builder
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

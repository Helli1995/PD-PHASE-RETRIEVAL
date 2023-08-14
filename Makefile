# Makefile for Makefile.pdlibbuilder.
#

lib.name := rtpghi

rtpghi~.class.sources = src/rtpghi~.c
#init_plan.class.sources = src/init_plan_backup.c

ldlibs += -lltfat -lphaseret


PDLIBBUILDER_DIR=./pd-lib-builder
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder


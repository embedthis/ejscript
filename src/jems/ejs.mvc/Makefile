#
#	Makefile for the ejs.mvc jem
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

TARGETS		+= $(BLD_BIN_DIR)/ejsmvc
ifeq ($(BLD_OS),WIN)
	TARGETS	+= $(BLD_BIN_DIR)/ejsmvc.cmd
endif

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/ejsmvc: ejsmvc
	$(call log) "[Generate]" "ejsmvc"
	ejsc ejsmvc ; # Just to do a compilation check
	cp ejsmvc $(BLD_BIN_DIR)/ejsmvc
	chmod 755 $(BLD_BIN_DIR)/ejsmvc

$(BLD_BIN_DIR)/ejsmvc.cmd: ejsmvc.cmd
	$(call log) "[Generate]" "ejsmvc.cmd"
	cp ejsmvc.cmd $(BLD_BIN_DIR)/ejsmvc.cmd
	chmod 755 $(BLD_BIN_DIR)/ejsmvc.cmd

#
#	Makefile for the ejs.mvc jem
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

TARGETS		+= $(BLD_BIN_DIR)/ejsmvc
TARGETS		+= $(BLD_BIN_DIR)/ejsmvc.cmd
TARGETS		+= $(BLD_LIB_DIR)/www

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

$(BLD_LIB_DIR)/www: Makefile www/*
	mkdir -p $(BLD_LIB_DIR)/www/images
	mkdir -p $(BLD_LIB_DIR)/www/themes
	cp -r www/* $(BLD_LIB_DIR)/www

cleanExtra:
	rm -rf $(BLD_LIB_DIR)/www

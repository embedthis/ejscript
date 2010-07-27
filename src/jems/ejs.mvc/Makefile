#
#	Makefile for the ejs.mvc jem
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

TARGETS		+= $(BLD_BIN_DIR)/mvc
TARGETS		+= $(BLD_BIN_DIR)/mvc.cmd
TARGETS		+= $(BLD_LIB_DIR)/www

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/mvc: mvc
	$(call log) "[Generate]" "mvc"
	ejsc mvc ; # Just to do a compilation check
	cp mvc $(BLD_BIN_DIR)/mvc
	chmod 755 $(BLD_BIN_DIR)/mvc

$(BLD_BIN_DIR)/mvc.cmd: mvc.cmd
	$(call log) "[Generate]" "mvc.cmd"
	cp mvc.cmd $(BLD_BIN_DIR)/mvc.cmd
	chmod 755 $(BLD_BIN_DIR)/mvc.cmd

$(BLD_LIB_DIR)/www: Makefile www/*
	mkdir -p $(BLD_LIB_DIR)/www/images
	mkdir -p $(BLD_LIB_DIR)/www/themes
	cp -r www/* $(BLD_LIB_DIR)/www

cleanExtra:
	rm -rf $(BLD_LIB_DIR)/www

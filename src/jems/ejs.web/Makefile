#
#	Makefile for the ejs.web jem
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

PRE_DIRS	+= src
TARGETS		+= $(BLD_LIB_DIR)/www

compileExtra: $(TARGETS)

$(BLD_LIB_DIR)/www: Makefile www/*
	mkdir -p $(BLD_LIB_DIR)/www/images
	mkdir -p $(BLD_LIB_DIR)/www/themes
	cp -r www/* $(BLD_LIB_DIR)/www

cleanExtra:
	rm -rf $(BLD_LIB_DIR)/www

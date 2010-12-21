#
#	Makefile for the ejs.template jem. Web page templating engine and template command.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

FILES		+= TemplateParser.es
TARGETS		+= $(BLD_BIN_DIR)/ejspage
TARGETS		+= $(BLD_BIN_DIR)/ejspage.cmd

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/ejspage: ejspage
	@$(call log) "[Update]" ejspage
	cp ejspage $(BLD_BIN_DIR)/ejspage
	chmod 755 $(BLD_BIN_DIR)/ejspage

$(BLD_BIN_DIR)/ejspage.cmd: ejspage.cmd
	cp ejspage.cmd $(BLD_BIN_DIR)/ejspage.cmd
	chmod 755 $(BLD_BIN_DIR)/ejspage.cmd

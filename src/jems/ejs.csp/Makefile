#
#	Makefile for the ejs.web.template jem. Web page templating engine and template command.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

FILES		+= TemplateParser.es
TARGETS		+= $(BLD_BIN_DIR)/ejspage
TARGETS		+= $(BLD_BIN_DIR)/ejspage.cmd

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/ejspage: ejspage.es
	@$(call log) "[Update]" ejspage.es
	cp ejspage.es $(BLD_BIN_DIR)/ejspage
	chmod 755 $(BLD_BIN_DIR)/ejspage

$(BLD_BIN_DIR)/ejspage.cmd: ejspage.cmd
	cp ejspage.cmd $(BLD_BIN_DIR)/ejspage.cmd
	chmod 755 $(BLD_BIN_DIR)/ejspage.cmd

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4
#

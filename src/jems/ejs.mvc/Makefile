#
#	Makefile for the ejs.mvc jem
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.mvc
MOD			= $(NAME).mod
JEM			= $(NAME)-$(BLD_VERSION)
TARGETS		+= $(BLD_BIN_DIR)/mvc $(BLD_BIN_DIR)/mvc.cmd
TARGETS		+= $(BLD_JEM_DIR)/$(JEM)

compileExtra: $(TARGETS)

module:

package.json: Makefile $(BLD_INC_DIR)/buildConfig.h
	ejs $(BLD_BIN_DIR)/jem --field version --value $(BLD_VERSION) edit

$(BLD_JEM_DIR)/$(JEM): $(BLD_BIN_DIR)/mvc package.json
	ejs $(BLD_BIN_DIR)/jem build
	ejs $(BLD_BIN_DIR)/jem --to $(BLD_JEM_DIR) publish $(JEM)
	rm -f $(JEM).jem

$(BLD_BIN_DIR)/mvc: mvc.es
	$(call log) "[Generate]" "mvc"
	ejsc mvc.es ; # Just to do a compilation check
	cp mvc.es $(BLD_BIN_DIR)/mvc
	chmod 755 $(BLD_BIN_DIR)/mvc

$(BLD_BIN_DIR)/mvc.cmd: mvc.cmd
	$(call log) "[Generate]" "mvc.cmd"
	cp mvc.cmd $(BLD_BIN_DIR)/mvc.cmd
	chmod 755 $(BLD_BIN_DIR)/mvc.cmd

cleanExtra:
	rm -fr $(BLD_JEM_DIR)/$(JEM)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

#
#	Makefile for the jem command
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.jem
JEM			= $(NAME)-$(BLD_VERSION)
TARGETS		+= $(BLD_BIN_DIR)/jem $(BLD_BIN_DIR)/jem.cmd
TARGETS		+= $(BLD_JEM_DIR)/$(JEM)

compileExtra: package.json $(TARGETS)

module:		dummy

package.json: Makefile $(BLD_INC_DIR)/buildConfig.h
	ejs $(BLD_BIN_DIR)/jem --field version --value $(BLD_VERSION) edit

$(BLD_JEM_DIR)/$(JEM): $(BLD_BIN_DIR)/jem
	ejs $(BLD_BIN_DIR)/jem build
	ejs $(BLD_BIN_DIR)/jem --to $(BLD_JEM_DIR) publish $(JEM)
	rm -f $(JEM).jem

$(BLD_BIN_DIR)/jem: jem.es
	$(call log) "[Generate]" jem
	cp jem.es $(BLD_BIN_DIR)/jem
	chmod 755 $(BLD_BIN_DIR)/jem

$(BLD_BIN_DIR)/jem.cmd: jem.cmd
	$(call log) "[Generate]" "jem.cmd"
	cp jem.cmd $(BLD_BIN_DIR)/jem.cmd
	chmod 755 $(BLD_BIN_DIR)/jem.cmd

cleanExtra:
	rm -fr $(BLD_JEM_DIR)/$(JEM)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

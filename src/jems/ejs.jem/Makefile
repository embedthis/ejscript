#
#	Makefile for the jem command
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.jem
JEM			= $(NAME)-$(BLD_VERSION)
TARGETS		+= $(BLD_BIN_DIR)/jem $(BLD_BIN_DIR)/jem.cmd
TARGETS		+= $(BLD_JEM_DIR)/$(JEM)

compileExtra: $(TARGETS)

#	$(BLD_MOD_DIR)/$(MOD): *.es
#		$(call log) [Compile] "$(NAME)"
#		rm -f *.mod *.slots.h
#		$(BLD_BUILD_BIN_DIR)/ejsc $(_ESFLAGS) *.es
#		$(BLD_BUILD_BIN_DIR)/ejsmod --cslots $(MOD)
#		diff $(NAME).slots.h $(BLD_INC_DIR) >/dev/null 2>&1 || cp $(NAME).slots.h $(BLD_INC_DIR) 
#		cp *.mod $(BLD_BUILD_MOD_DIR) ; mv *.mod $(BLD_MOD_DIR)
#		rm -f $(NAME).slots.h

$(BLD_JEM_DIR)/$(JEM): $(BLD_BIN_DIR)/jem package.json
	jem --field version --value $(BLD_VERSION) edit
	jem build
	jem --to $(BLD_JEM_DIR) publish $(JEM)
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

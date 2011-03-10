#
#	Makefile for the ejs.tar - Tar archive format support
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.tar
MOD			= $(NAME).mod
JEM			= $(NAME)-$(BLD_VERSION)
MODULE		+= $(BLD_MOD_DIR)/$(NAME).mod
TARGETS		+= $(MODULE) $(BLD_JEM_DIR)/$(JEM)

compileExtra: $(TARGETS)

module: $(MODULE)

$(BLD_MOD_DIR)/$(MOD): *.es $(BLD_MOD_DIR)/ejs.mod
	$(call log) [Compile] "$(MOD)"
	rm -f *.mod *.slots.h $(BLD_BUILD_MOD_DIR)/$(MOD) $(BLD_MOD_DIR)/$(MOD)
	$(BLD_BUILD_BIN_DIR)/ejsc $(_ESFLAGS) *.es
	$(BLD_BUILD_BIN_DIR)/ejsmod --cslots $(MOD)
	diff $(NAME).slots.h $(BLD_INC_DIR) >/dev/null 2>&1 || cp $(NAME).slots.h $(BLD_INC_DIR) 
	cp *.mod $(BLD_BUILD_MOD_DIR) ; mv *.mod $(BLD_MOD_DIR)
	rm -f $(NAME).slots.h

$(BLD_JEM_DIR)/$(JEM): $(BLD_MOD_DIR)/$(MOD) package.json
	jem --field version --value $(BLD_VERSION) edit
	jem setdeps $(BLD_MOD_DIR)/$(MOD)
	jem build
	jem --to $(BLD_JEM_DIR) publish $(JEM)
	rm -f $(JEM).jem

cleanExtra:
	rm -fr $(BLD_JEM_DIR)/$(JEM)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

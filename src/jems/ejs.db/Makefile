#
#	Makefile for the ejs.db - Database support
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.db
MOD			= $(NAME).mod
JEM			= $(NAME)-$(BLD_VERSION)
MODULE		+= $(BLD_LIB_DIR)/$(NAME).mod 
TARGETS		+= $(MODULE) $(BLD_JEM_DIR)/$(JEM)

compileExtra: $(TARGETS)

module: $(MODULE)

$(BLD_LIB_DIR)/$(MOD): *.es $(BLD_LIB_DIR)/ejs.mod
	$(call log) [Compile] "$(MOD)"
	rm -f *.mod *.slots.h $(BLD_BUILD_LIB_DIR)/$(MOD) $(BLD_LIB_DIR)/$(MOD)
	$(BLD_BUILD_BIN_DIR)/ejsc $(_ESFLAGS) *.es
	$(BLD_BUILD_BIN_DIR)/ejsmod --cslots $(MOD)
	diff $(NAME).slots.h $(BLD_INC_DIR) >/dev/null 2>&1 || cp $(NAME).slots.h $(BLD_INC_DIR) 
	cp *.mod $(BLD_BUILD_LIB_DIR) ; mv *.mod $(BLD_LIB_DIR)
	rm -f $(NAME).slots.h

package.json: Makefile $(BLD_INC_DIR)/buildConfig.h
	jem --field version --value $(BLD_VERSION) edit

$(BLD_JEM_DIR)/$(JEM): $(BLD_LIB_DIR)/$(MOD) package.json
	jem setdeps $(BLD_LIB_DIR)/$(MOD)
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

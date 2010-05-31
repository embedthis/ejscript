#
#	Makefile for the ejs.cache jem
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

VJEM		= ejs.cache-$(BLD_VERSION)
TARGET		+= $(BLD_JEM_DIR)/$(VJEM)/$(VJEM).jem
SRC			+= *.es
DOC_DIR		:= doc

compileExtra: 
	
x: $(TARGET)

$(TARGET): $(VJEM).jem
	$(call log) "[Publish]" "jem --to $(BLD_JEM_DIR) publish $(VJEM)"
	jem --to $(BLD_JEM_DIR) publish $(VJEM)

$(VJEM).jem: $(SRc)
	jem --field version --value $(BLD_VERSION) edit
	# jem setdeps $(BLD_MOD_DIR)/$(VJEM).mod
	$(call log) "[Building]" "jem build"
	jem build


$(DOC_DIR)/index.html: Makefile $(SRC)
	$(call log) "[Generate]" documentation
	$(call setlibpath) ; ejsc --doc --bind $(_ESFLAGS) --out .doc.mod $(SRC)
	mkdir -p $(DOC_DIR)
	rm -rf $(DOC_DIR)/*.html $(DOC_DIR)/*.css $(DOC_DIR)/images/*
	$(call setlibpath) ; ejsmod --warn --html $(DOC_DIR) --require "" .doc.mod
	rm -f .doc.mod

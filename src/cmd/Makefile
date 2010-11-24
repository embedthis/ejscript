#
#	Makefile for the Ejscript commands
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

TOOLS		+= $(BLD_BIN_DIR)/ejsc$(BLD_EXE)
TOOLS		+= $(BLD_BIN_DIR)/ejsmod$(BLD_EXE) 
TARGETS		+= $(BLD_BIN_DIR)/ejs$(BLD_EXE) 

ifeq	($(BLD_CC_EDITLINE),1)
	LIBS		+= edit curses
endif

compileExtra: tools $(TARGETS)

tools: $(TOOLS)

EJS_OBJECTS		+= $(BLD_OBJ_DIR)/ejs$(BLD_OBJ)
ifeq ($(BLD_FEATURE_ROMFS),1)
	EJS_OBJECTS	+= $(BLD_OBJ_DIR)/romFiles$(BLD_OBJ)
compileFirst: romFiles.c
endif

#
#	This is just a sample for how to build a ROMable version of ejs
#
romFiles.c:
	$(call log) "[Generate]" "romFiles.c"
	echo "print('Hello World')" >hello.es
	echo "hello.es" >rom.files 
	$(call setlibpath) ; sort rom.files | uniq | xargs $(BLD_BIN_DIR)/makerom --name romFiles >romFiles.c
	rm hello.es

#
#	Make the compiler
#
$(BLD_BIN_DIR)/ejsc$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/ejsc$(BLD_EXE) --search "$(BLD_EJS_LIBPATHS)" --libs "$(BLD_EJS_LIBS)" \
		"$(BLD_OBJ_DIR)/ejsc$(BLD_OBJ)"

#
#	Make the Ejscript shell
#
$(BLD_BIN_DIR)/ejs$(BLD_EXE): $(OBJECTS) 
	bld --exe $(BLD_BIN_DIR)/ejs$(BLD_EXE) --search "$(BLD_EJS_LIBPATHS)" --libs "$(BLD_EJS_LIBS) $(LIBS)" $(EJS_OBJECTS)

#
#	Make the ejsmod (module manager)
#
$(BLD_BIN_DIR)/ejsmod$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/ejsmod$(BLD_EXE) --search "$(BLD_EJS_LIBPATHS)" --libs "$(BLD_EJS_LIBS)" \
		ejsmod doc docFiles listing slotGen

docFiles.c: images/banner.jpg doc.css images/inherit.gif
	$(call setlibpath) ; makerom images/banner.jpg doc.css images/inherit.gif | \
		sed 's/BLD_FEATURE_ROMFS/1/;s/mpr.h/ejsmod.h/;s/MprRomInode defaultRomFiles/DocFile docFiles/' >docFiles.c

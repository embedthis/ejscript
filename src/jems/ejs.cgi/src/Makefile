#
#	Makefile for the Escript CGI handler
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#
#
include 		.makedep

ifeq ($(BUILD_CROSS_OR_COMPLETE_NATIVE),1)
	TARGETS    += $(BLD_BIN_DIR)/ejscgi$(BLD_EXE)
endif

compileExtra:	$(TARGETS)

$(BLD_BIN_DIR)/ejscgi$(BLD_EXE): $(OBJECTS)
	bld --shared --exe $(BLD_BIN_DIR)/ejscgi$(BLD_EXE) --libs "$(BLD_EJS_LIBS)" \
		--modules "ejs.web$(BLD_SHOBJ)" $(BLD_OBJ_DIR)/ejsCgiHandler$(BLD_OBJ)



#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

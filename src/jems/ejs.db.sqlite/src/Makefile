#
#	Makefile to build the SQLite Classe
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 			.makedep

JEM				:= ejs.db.sqlite
MAKE_IFLAGS		+= $(BLD_SQLITE_IFLAGS) $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)
ifeq ($(BLD_HOST_WIN),1)
	MAKE_DFLAGS	+= -D_CRT_SECURE_NO_WARNINGS
endif
ifeq ($(BLD_HOST_OS),CYGWIN)
	_CFLAGS 	:= $(shell echo $(_CFLAGS) | sed 's/-W3/-W1/;s/-Wall/-w/;s/-g3//')
else
	_CFLAGS 	:= $(shell echo $(_CFLAGS) | sed 's/-W3/-W1/;s/-Wall/-w/')
endif

ifeq ($(BLD_FEATURE_SQLITE),1)
	TARGETS		+= $(BLD_LIB_DIR)/libsqlite3$(BLD_LIB)
	TARGETS		+= $(BLD_LIB_DIR)/$(JEM)$(BLD_LIB)
	TARGETS		+= $(BLD_BIN_DIR)/ejssql$(BLD_EXE)
endif

compileExtra:   $(TARGETS)

$(BLD_LIB_DIR)/$(JEM)$(BLD_LIB): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --library $(BLD_LIB_DIR)/$(JEM)$(BLD_LIB) --libs "ejs $(BLD_MPR_LIBS) $(BLD_SQLITE_LIBS)" \
        $(BLD_OBJ_DIR)/ejsSqlite$(BLD_OBJ)

$(BLD_LIB_DIR)/libsqlite3$(BLD_LIB): $(BLD_OBJ_DIR)/sqlite3$(BLD_OBJ)
	bld --library $(BLD_LIB_DIR)/libsqlite3 --libs "$(BLD_MPR_LIBS)" sqlite3

$(BLD_BIN_DIR)/ejssql$(BLD_EXE): $(BLD_LIB_DIR)/libsqlite3$(BLD_LIB)
	bld --exe $(BLD_BIN_DIR)/ejssql$(BLD_EXE) --libs "$(BLD_MPR_LIBS)" ejssql sqlite3



#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

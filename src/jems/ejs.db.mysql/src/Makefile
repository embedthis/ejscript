#
#	Makefile to build the Mysql Classes
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 			.makedep

JEM				:= ejs.db.mysql
MAKE_IFLAGS		+= $(BLD_MYSQL_IFLAGS)
ifeq ($(BLD_HOST_WIN),1)
	MAKE_DFLAGS	+= -D_CRT_SECURE_NO_WARNINGS
endif
ifeq ($(BLD_HOST_OS),CYGWIN)
	_CFLAGS 	:= $(shell echo $(_CFLAGS) | sed 's/-W3/-W1/;s/-Wall/-w/;s/-g3//')
else
	_CFLAGS 	:= $(shell echo $(_CFLAGS) | sed 's/-W3/-W1/;s/-Wall/-w/')
endif

ifeq ($(BLD_FEATURE_MYSQL),1)
	TARGETS		+= $(BLD_LIB_DIR)/libmysql$(BLD_LIB)
	TARGETS		+= $(BLD_MOD_DIR)/$(JEM)$(BLD_LIB)
endif

compileExtra:   $(TARGETS)

$(BLD_MOD_DIR)/$(JEM)$(BLD_LIB): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --library $(BLD_MOD_DIR)/$(JEM)$(BLD_LIB) --libs "ejs $(BLD_MPR_LIBS) $(BLD_MYSQL_LIBS)" \
        $(BLD_OBJ_DIR)/ejsSqlite$(BLD_OBJ)

$(BLD_LIB_DIR)/libmysql$(BLD_LIB): $(BLD_OBJ_DIR)/mysql$(BLD_OBJ)
	bld --library $(BLD_LIB_DIR)/libmysql --libs "$(BLD_MPR_LIBS)" mysql

$(BLD_BIN_DIR)/ejssql$(BLD_EXE): $(BLD_LIB_DIR)/libmysql$(BLD_LIB)
	bld --exe $(BLD_BIN_DIR)/ejssql$(BLD_EXE) --libs "$(BLD_MPR_LIBS)" ejssql mysql



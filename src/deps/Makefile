#
#	Makefile for the Ejscript dependencies.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

ifeq ($(BLD_MPR_BUILTIN),1)
	PRE_DIRS    += mpr
else
	CLOBBER		+= mpr/*.c mpr/*.h
	IMPORTS     += $(BLD_MPR_IMPORTS) $(BLD_SSL_IMPORTS)
endif

ifeq ($(BLD_PCRE_BUILTIN),1)
	PRE_DIRS    += pcre
else
	CLOBBER		+= pcre/*.c pcre/*.h
	IMPORTS     += $(BLD_PCRE_IMPORTS)
endif

ifeq ($(BLD_HTTP_BUILTIN),1)
	PRE_DIRS    += http
else
	CLOBBER		+= http/*.c http/*.h
	IMPORTS     += $(BLD_HTTP_IMPORTS)
endif

IMPORTED    	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(IMPORTS)))

compileFirst:	$(IMPORTED)

$(IMPORTED): $(IMPORTS)
	getlib $^

clobberExtra:
	rm -f $(CLOBBERIT)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

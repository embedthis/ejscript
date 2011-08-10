#
#	Makefile to build the evalScript sample
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

EXPORT_OBJECTS	:= 0

include 		.makedep

MAKE_IFLAGS		:= $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)

compileExtra: main$(BLD_EXE)

main$(BLD_EXE) : $(OBJECTS)
	bld --exe main$(BLD_EXE) --rpath "$(BLD_LIB_DIR)" --libs "$(BLD_EJSCRIPT_LIBS)" $(OBJECTS)

cleanExtra:
	rm -f main$(BLD_EXE)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

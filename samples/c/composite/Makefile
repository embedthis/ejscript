#
#	Makefile to build the native sample
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

EXPORT_OBJECTS	:= 0

include 		.makedep

MAKE_IFLAGS		:= $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)

compileFirst: 	sample.mod

sample.mod: shape.es
	$(call setlibpath) ; ejsc shape.es
	$(call setlibpath) ; ejsmod --cslots sample.mod

compileExtra: sample.mod sample$(BLD_SHOBJ)

sample$(BLD_SHOBJ) : $(OBJECTS) $(BLD_EJS_MAKEDEP) sample.slots.h
	bld --shared --library sample --shared --search "$(BLD_LIB_DIR)" --libs "$(BLD_EJS_LIBS)" $(OBJECTS)

cleanExtra:
	rm -f *.lst *.mod *$(BLD_LIB)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

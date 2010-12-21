#
#	Makefile to build the composite sample
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

EXPORT_OBJECTS	:= 0
BUILD_CROSS		:= 0

include 		.makedep

ifeq			($(BLD_STATIC),0)
compileFirst: 	sample.mod

sample.mod: shape.es
	$(call setlibpath) ; ejsc shape.es
	$(call setlibpath) ; ejsmod --cslots sample.mod

compileExtra: sample.mod sample$(BLD_SHOBJ)

sample$(BLD_SHOBJ) : $(OBJECTS) $(BLD_EJS_MAKEDEP) sample.slots.h
	@bld --shared --library sample --shared --search "$(BLD_LIB_DIR) $(BLD_LIB_PREFIX)" \
		--libs "$(BLD_EJS_LIBS)" $(OBJECTS)

else
compileExtra:
	@$(call log) "[Notice]" "Skip building composite sample. Requires \"configure --enable-shared\"."
endif

cleanExtra:
	@rm -f *.lst *.mod *$(BLD_LIB)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

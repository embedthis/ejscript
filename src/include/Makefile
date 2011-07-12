#
#	Makefile for master includes
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#
MASTERS     += $(patsubst master/%.h,$(BLD_INC_DIR)/%.h,$(shell find master/*.slots.h))

include 	.makedep

compileExtra: $(MASTERS)

dependExtra:
	cp master/*slots.h $(BLD_INC_DIR)

cleanExtra:
	mkdir -p $(BLD_INC_DIR)
	rm -f $(BLD_INC_DIR)/ejs.*.h

.SUFFIXES: .h

$(BLD_INC_DIR)/%.h: master/%.h
	cp $< $@

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

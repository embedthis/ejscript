#
#	Makefile for the bit command
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.bit
TARGETS		+= $(BLD_BIN_DIR)/bit.es \
			   $(BLD_BIN_DIR)/bit$(BLD_EXE) \
			   $(BLD_LIB_DIR)/bits

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/bit.es: bit.es
	$(call log) "[Generate]" bit.es
	cp bit.es $(BLD_BIN_DIR)/bit.es

$(BLD_BIN_DIR)/bit$(BLD_EXE): $(BLD_BIN_DIR)/ejsrun$(BLD_EXE)
	$(call log) "[Create]" "bit$(BLD_EXE)"
	cp $(BLD_BIN_DIR)/ejsrun$(BLD_EXE) $(BLD_BIN_DIR)/bit$(BLD_EXE)
	chmod 755 $(BLD_BIN_DIR)/bit$(BLD_EXE)

$(BLD_LIB_DIR)/bits: bits bits/* bits/*/*
	$(call log) "[Export]" "bits"
	rm -fr $(BLD_LIB_DIR)/bits
	cp -r bits $(BLD_LIB_DIR)

cleanExtra:
	rm -fr $(BLD_LIB_DIR)/bits

manual:
	rm -fr $(BLD_TOP)/xcode/lib/bits
	cp -r bits $(BLD_TOP)/xcode/lib
	cp bit.es $(BLD_TOP)/xcode/bin
	echo Copied to xcode

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

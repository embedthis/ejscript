#
#	Makefile for the ejs core 
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

LISTING			+= --listing
POST_DIRS		+= src
EJS_MOD			+= $(call mod, ejs)
EJS_FILES		+= $(shell echo *.es | sort)

compileExtra: modules

modules: $(EJS_MOD)

#
#	Modules are (currently) cross-platform. If cross-compiling, BLD_LIB_DIR and BLD_BUILD_LIB_DIR are different.
#	Copy the module to both places. No harm if they are the same.
#
$(EJS_MOD): $(EJS_FILES)
	$(call log) [Compile] "Core Ejs Classes"
	rm -f *.mod *.slots.h $(BLD_BUILD_LIB_DIR)/*.mod $(BLD_LIB_DIR)/*.mod
	ejsc $(_ESFLAGS) --out ejs.mod --bind --require "" $(EJS_FILES)
	if [ "$(EJS_MOD_DIR)" != "" ] ; then \
		cp *.mod $(EJS_MOD_DIR) ; \
	else \
		cp *.mod $(BLD_BUILD_LIB_DIR) ; mv *.mod $(BLD_LIB_DIR) ; \
	fi
	$(call log) [Generate] "Core listings and slot files"
	ejsmod --require "" $(LISTING) --cslots $(call mod, ejs)
	changed=0 ; \
	for f in *.slots.h ; do \
		if [ -f $(BLD_INC_DIR)/$$f ] ; then \
			if ! diff $$f $(BLD_INC_DIR)/$$f >/dev/null 2>&1; then \
				printf "%12s %s\n" "[Install]" $$f ; \
				cp $$f $(BLD_INC_DIR) ; \
				changed=1 ; \
			fi ; \
		fi ; \
		rm -f $$f; \
	done ; \
	if [ "$$changed" = 1 -a "$(RETRY)" = "" ] ; then \
		echo ; \
		$(call log) [Notice] "Rebuild with Modified Slot Definitions" ; \
		$(MAKE) TRACE=$(TRACE) RETRY=1 $(MAKECMDGOALS) || exit 255 ; \
	fi

cleanExtra:
	find . -name '*.lst' -o -name '*.mod' -o -name '*.jem' -type f | xargs rm -f
	rm -f *.slots.h $(BLD_LIB_DIR)/ejs.slots.h
	rm -f $(BLD_LIB_DIR)/ejs.mod

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

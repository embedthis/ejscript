#
#	Makefile -- Top level Makefile for Ejscript
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#
#
#	Standard Make targets supported are:
#	
#		make 						# Does a "make compile"
#		make clean					# Removes generated objects
#		make compile				# Compiles the source
#		make depend					# Generates the make dependencies
#		make test 					# Runs unit tests
#		make package				# Creates an installable package
#
#	Installation targets. Use "make ROOT_DIR=myDir" to do a custom local install:
#
#		make install				# Call install-binary + install-dev
#		make install-binary			# Install binary files
#		make install-dev			# Install development libraries and headers
#
#	To remove, use make uninstall-ITEM, where ITEM is a component above.

include			build/make/Makefile.top
include			build/make/Makefile.ejs

testCleanup:
	echo killall testAppweb >/dev/null 2>&1 ; true

ifneq	($(BUILDING_CROSS),1)
testExtra: 
	$(BLD_BIN_DIR)/ejs $(BLD_TOOLS_DIR)/utest -v src
endif

diff import sync:
	if [ ! -x $(BLD_TOOLS_DIR)/edep$(BLD_BUILD_EXE) -a "$(BUILDING_CROSS)" != 1 ] ; then \
		$(MAKE) -S --no-print-directory _RECURSIVE_=1 -C $(BLD_TOP)/build/src compile ; \
	fi
	if [ "`git branch`" != "* master" ] ; then echo "Sync only in default branch" ; echo 255 ; fi
	import.ksh --$@ --src ../tools --dir . ../tools/build/export/export.gen
	import.ksh --$@ --src ../tools --dir . ../tools/build/export/export.configure
	import.ksh --$@ --src ../mpr 	--dir . ../mpr/build/export/export.gen
	import.ksh --$@ --src ../mpr 	--dir ./src/include --strip ./all/ ../mpr/build/export/export.h
	import.ksh --$@ --src ../mpr 	--dir ./src/deps/mpr --strip ./all/ ../mpr/build/export/export.c
	import.ksh --$@ --src ../http 	--dir . ../http/build/export/export.gen
	import.ksh --$@ --src ../http 	--dir ./src/include --strip ./all/ ../http/build/export/export.h
	import.ksh --$@ --src ../http 	--dir ./src/deps/http --strip ./all/ ../http/build/export/export.c
	import.ksh --$@ --src ../pcre 	--dir . ../pcre/build/export/export.gen
	import.ksh --$@ --src ../pcre 	--dir ./src/include --strip ./all/ ../pcre/build/export/export.h
	import.ksh --$@ --src ../pcre 	--dir ./src/deps/pcre --strip ./all/ ../pcre/build/export/export.c
	import.ksh --$@ --src ../appweb --dir ./src/include --strip ./all/ ../appweb/build/export/export.h
	import.ksh --$@ --src ../appweb --dir ./src/deps/appweb --strip ./all/ ../appweb/build/export/export.c
	echo


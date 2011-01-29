#
#	Makefile -- Top level Makefile for Ejscript
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
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

BUILD_DEPTH	?= 2

include	    build/make/Makefile.top
include		build/make/Makefile.ejs

ifeq	($(BLD_CROSS),0)
DISABLEtestExtra: 
	$(BLD_BIN_DIR)/ejs $(BLD_TOOLS_DIR)/utest -v -d $(BUILD_DEPTH) src
endif

diff import sync:
	import.sh --$@ ../tools/out/releases/tools-dist.tgz
	import.sh --$@ ../mpr/out/releases/mpr-dist.tgz
	import.sh --$@ ../pcre/out/releases/pcre-dist.tgz
	import.sh --$@ ../http/out/releases/http-dist.tgz

compileFinal:
	make dist

DISABLEtestExtra: test-projects

test-projects:
ifeq    ($(BLD_HOST_OS),WIN)
	if [ "$(BUILD_DEPTH)" -ge 3 ] ; then \
		$(BLD_TOOLS_DIR)/nativeBuild ; \
	fi
endif

ext:
	./configure --with-mpr=../mpr --with-ssl=../mpr --with-http=../http --with-pcre=../pcre

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

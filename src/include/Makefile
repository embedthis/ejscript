#
#	Makefile for master includes
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

#
#   Update buildConfig.h version information. This is used in windows packages for building with VS.
#
dependExtra:
	for f in *slots* ; do \
		cp $$f .. ; \
	done
ifeq ($(BLD_OS),WIN)
	cat buildConfig.h | egrep -v 'BLD_VERSION|BLD_NUMBER|BLD_.*_VERSION|BLD_VNUM' | grep -v BLD_CC_CL_VERSION >versions.h
	cat ../buildConfig.h | egrep 'BLD_VERSION|BLD_NUMBER|BLD_.*_VERSION|BLD_VNUM' | grep -v BLD_CC_CL_VERSION >>versions.h
	mv versions.h buildConfig.h
	rm -f versions.h
endif

cleanExtra: dependExtra

import:
	@for f in *slots* ; do \
		echo cp ../$$f . ; \
		cp ../$$f . ; \
	done


#
#	Makefile - Makefile to build Ejscript with bit.
#
#	This Makefile will build a "minimal" Ejscript without external packages.
#	It is used to build Ejscript the first time before bit is available.
#	Once built, use bit to configure and rebuild as required:
#		bit configure build
#	

ARCH 	:= $(shell uname -m)
PROFILE	:= debug
UNAME 	:= $(shell uname)

ifeq ($(UNAME),Darwin)
	OS	:=	macosx
endif
ifeq ($(UNAME),Linux)
	OS	:=	linux
endif
ifeq ($(UNAME),Solaris)
	OS	:=	SOLARIS
endif
ifeq ($(UNAME),Cygwin)
	OS	:=	win
endif

all compile:
	make -f projects/$(OS)-$(ARCH)-$(PROFILE).mk $@
	@echo ; echo 'You can now use "bit" to configure and re-build Ejscript.'
	@echo 'Run "bit configure build"'

build configure generate test package:
	@bit $@

clean clobber:
	make -f projects/$(OS)-$(ARCH)-$(PROFILE).mk $@

version:
	@bit -q version

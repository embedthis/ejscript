#
#	Makefile - Build and install the TestMe project
#
MFLAGS := --no-print-directory

all: build tidy

#
#  Install required dependencies
#
prep:
	bun install

#
#  Build the project. Builds the tm binary and the support files.
#
build:
	bun run build

test: build
	bun test
	# tm test

install: build
	@echo Installing via bun link
	bun link
	pak cache 

clean: tidy

#
#  Remove bun artifacts
#
tidy:
	@rm -f .*.bun-build

.PHONY: prep run build test install tidy clean

LOCAL_MAKEFILE := $(strip $(wildcard ./.local.mk))

ifneq ($(LOCAL_MAKEFILE),)
include $(LOCAL_MAKEFILE)
endif

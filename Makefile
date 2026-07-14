#
#	Makefile - Build and install the Ejscript project
#
MFLAGS := --no-print-directory

all: build

#
#  Install required dependencies
#
prep:
	bun install

#
#  Build the project. Compiles TypeScript to dist/ with type definitions.
#
build:
	bun run build

test: build
	@bin/prep-test.sh
	tm test

install: build
	@echo Installing via bun link
	bun --silent link
	pak -f cache

#
#  Publish the package to npm. Depends on test so the suite must pass first:
#  package.json prepublishOnly only runs typecheck and build, as tm requires
#  the prep-test.sh prerequisites that only make provides.
#  Public access comes from publishConfig in package.json.
#  Requires an authenticated npm login.
#
promote: test
	npm publish

clean: tidy

#
#  Remove bun artifacts
#
tidy:
	@rm -f .*.bun-build

.PHONY: prep run build test install promote tidy clean

LOCAL_MAKEFILE := $(strip $(wildcard ./.local.mk))

ifneq ($(LOCAL_MAKEFILE),)
include $(LOCAL_MAKEFILE)
endif

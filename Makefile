# Hiredis Makefile
# Copyright (C) 2010-2011 Salvatore Sanfilippo <antirez at gmail dot com>
# Copyright (C) 2010-2011 Pieter Noordhuis <pcnoordhuis at gmail dot com>
# This file is released under the BSD license, see the COPYING file

OBJ=adlist.o command.o crc16.o hiarray.o hircluster.o hiutil.o
LIBNAME=libhiredis_vip
PKGCONFNAME=hiredis-vip.pc

HIREDIS_VIP_MAJOR=$(shell grep HIREDIS_VIP_MAJOR hircluster.h | awk '{print $$3}')
HIREDIS_VIP_MINOR=$(shell grep HIREDIS_VIP_MINOR hircluster.h | awk '{print $$3}')
HIREDIS_VIP_PATCH=$(shell grep HIREDIS_VIP_PATCH hircluster.h | awk '{print $$3}')
HIREDIS_VIP_SONAME=$(shell grep HIREDIS_VIP_SONAME hircluster.h | awk '{print $$3}')

# Installation related variables and target
PREFIX?=/usr/local
INCLUDE_PATH?=include/hiredis-vip
LIBRARY_PATH?=lib
PKGCONF_PATH?=pkgconfig
INSTALL_INCLUDE_PATH= $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH= $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)
INSTALL_PKGCONF_PATH= $(INSTALL_LIBRARY_PATH)/$(PKGCONF_PATH)

# Fallback to gcc when $CC is not in $PATH.
CC:=$(shell sh -c 'type $${CC%% *} >/dev/null 2>/dev/null && echo $(CC) || echo gcc')
CXX:=$(shell sh -c 'type $${CXX%% *} >/dev/null 2>/dev/null && echo $(CXX) || echo g++')
OPTIMIZATION?=-O3
WARNINGS=-Wall -W -Wstrict-prototypes -Wwrite-strings -Wno-missing-field-initializers
DEBUG_FLAGS?= -g -ggdb
REAL_CFLAGS=$(OPTIMIZATION) -fPIC $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $(DEBUG_FLAGS)
REAL_LDFLAGS=$(LDFLAGS)

DYLIBSUFFIX=so
STLIBSUFFIX=a
DYLIB_MINOR_NAME=$(LIBNAME).$(DYLIBSUFFIX).$(HIREDIS_VIP_SONAME)
DYLIB_MAJOR_NAME=$(LIBNAME).$(DYLIBSUFFIX).$(HIREDIS_VIP_MAJOR)
DYLIBNAME=$(LIBNAME).$(DYLIBSUFFIX)
SSL_DYLIBNAME=$(SSL_LIBNAME).$(DYLIBSUFFIX)
DYLIB_MAKE_CMD=$(CC) -shared -Wl,-soname,$(DYLIB_MINOR_NAME)
STLIBNAME=$(LIBNAME).$(STLIBSUFFIX)
SSL_STLIBNAME=$(SSL_LIBNAME).$(STLIBSUFFIX)
STLIB_MAKE_CMD=$(AR) rcs

# Platform-specific overrides
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

all: $(DYLIBNAME) $(STLIBNAME) $(PKGCONFNAME)

# Deps (use make dep to generate this)
adlist.o: adlist.c adlist.h hiutil.h
command.o: command.c command.h hiredis.h read.h sds.h adlist.h hiutil.h hiarray.h
crc16.o: crc16.c hiutil.h
hiarray.o: hiarray.c hiutil.h hiarray.h
hircluster.o: hircluster.c fmacros.h win32.h hircluster.h hiredis.h read.h sds.h async.h hiutil.h adlist.h hiarray.h command.h dict.c dict.h
hiutil.o: hiutil.c win32.h hiutil.h

$(DYLIBNAME): $(OBJ)
	$(DYLIB_MAKE_CMD) -o $(DYLIBNAME) $(OBJ) $(REAL_LDFLAGS)

$(STLIBNAME): $(OBJ)
	$(STLIB_MAKE_CMD) $(STLIBNAME) $(OBJ)

dynamic: $(DYLIBNAME)
static: $(STLIBNAME)

.c.o:
	$(CC) -std=c99 -pedantic -c $(REAL_CFLAGS) $<

clean:
	rm -rf $(DYLIBNAME) $(STLIBNAME) $(PKGCONFNAME) *.o *.gcda *.gcno *.gcov

dep:
	$(CC) $(CPPFLAGS) $(CFLAGS) -MM *.c

INSTALL?= cp -pPR

$(PKGCONFNAME): hircluster.h
	@echo "Generating $@ for pkgconfig..."
	@echo prefix=$(PREFIX) > $@
	@echo exec_prefix=\$${prefix} >> $@
	@echo libdir=$(PREFIX)/$(LIBRARY_PATH) >> $@
	@echo includedir=$(PREFIX)/$(INCLUDE_PATH) >> $@
	@echo >> $@
	@echo Name: hiredis-vip >> $@
	@echo Description: Minimalistic C client library for Redis with cluster support. >> $@
	@echo Version: $(HIREDIS_VIP_MAJOR).$(HIREDIS_VIP_MINOR).$(HIREDIS_VIP_PATCH) >> $@
	@echo Libs: -L\$${libdir} -lhiredis-vip >> $@
	@echo Cflags: -I\$${includedir} -D_FILE_OFFSET_BITS=64 >> $@

install: $(DYLIBNAME) $(STLIBNAME) $(PKGCONFNAME)
	mkdir -p $(INSTALL_INCLUDE_PATH) $(INSTALL_INCLUDE_PATH)/adapters $(INSTALL_LIBRARY_PATH)
	$(INSTALL) hiredis.h async.h read.h sds.h hircluster.h adlist.h dict.h hiarray.h $(INSTALL_INCLUDE_PATH)
	$(INSTALL) adapters/*.h $(INSTALL_INCLUDE_PATH)/adapters
	$(INSTALL) $(DYLIBNAME) $(INSTALL_LIBRARY_PATH)/$(DYLIB_MINOR_NAME)
	cd $(INSTALL_LIBRARY_PATH) && ln -sf $(DYLIB_MINOR_NAME) $(DYLIBNAME)
	$(INSTALL) $(STLIBNAME) $(INSTALL_LIBRARY_PATH)
	mkdir -p $(INSTALL_PKGCONF_PATH)
	$(INSTALL) $(PKGCONFNAME) $(INSTALL_PKGCONF_PATH)

32bit:
	@echo ""
	@echo "WARNING: if this fails under Linux you probably need to install libc6-dev-i386"
	@echo ""
	$(MAKE) CFLAGS="-m32" LDFLAGS="-m32"

32bit-vars:
	$(eval CFLAGS=-m32)
	$(eval LDFLAGS=-m32)

gprof:
	$(MAKE) CFLAGS="-pg" LDFLAGS="-pg"

gcov:
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs"

coverage: gcov
	make check
	mkdir -p tmp/lcov
	lcov -d . -c -o tmp/lcov/hiredis.info
	genhtml --legend -o tmp/lcov/report tmp/lcov/hiredis.info

noopt:
	$(MAKE) OPTIMIZATION=""

.PHONY: all test check clean dep install 32bit 32bit-vars gprof gcov noopt

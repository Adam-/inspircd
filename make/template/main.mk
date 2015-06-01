%target BSD_MAKE BSDmakefile
%target GNU_MAKE GNUmakefile
#
# InspIRCd -- Internet Relay Chat Daemon
#
#   Copyright (C) 2009-2010 Daniel De Graaf <danieldg@inspircd.org>
#
# This file is part of InspIRCd.  InspIRCd is free software: you can
# redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, version 2.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


#
#               InspIRCd Main Makefile
#
# This file is automagically generated by configure, from
# make/template/main.mk. Any changes made to the generated
#     files will go away whenever it is regenerated!
#
# Please do not edit unless you know what you're doing. This
# needs to work in both GNU and BSD make; it is mangled for
# them by configure.
#


CXX = @CXX@
COMPILER = @COMPILER_NAME@
SYSTEM = @SYSTEM_NAME@
BUILDPATH ?= $(PWD)/build
SOCKETENGINE = @SOCKETENGINE@
CORECXXFLAGS = -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -pipe -Iinclude -Wall -Wextra -Wfatal-errors -Wno-unused-parameter -Wshadow
LDLIBS = -lstdc++
CORELDFLAGS = -rdynamic -L. $(LDFLAGS)
PICLDFLAGS = -fPIC -shared -rdynamic $(LDFLAGS)
BASE = "$(DESTDIR)@BASE_DIR@"
CONPATH = "$(DESTDIR)@CONFIG_DIR@"
MANPATH = "$(DESTDIR)@MANUAL_DIR@"
MODPATH = "$(DESTDIR)@MODULE_DIR@"
DATPATH = "$(DESTDIR)@DATA_DIR@"
BINPATH = "$(DESTDIR)@BINARY_DIR@"
INSTALL = install
INSTUID = @UID@
INSTMODE_DIR = 0750
INSTMODE_BIN = 0750
INSTMODE_LIB = 0640

@IFNEQ $(COMPILER) ICC
  CORECXXFLAGS += -Woverloaded-virtual -Wshadow
@IFNEQ $(SYSTEM) openbsd
    CORECXXFLAGS += -pedantic -Wformat=2 -Wmissing-format-attribute
@ENDIF

@IFNEQ $(SYSTEM) darwin
  LDLIBS += -pthread
@ENDIF

@IFEQ $(SYSTEM) linux
  LDLIBS += -ldl -lrt
@ENDIF
@IFEQ $(SYSTEM) gnukfreebsd
  LDLIBS += -ldl -lrt
@ENDIF
@IFEQ $(SYSTEM) gnu
  LDLIBS += -ldl -lrt
@ENDIF
@IFEQ $(SYSTEM) solaris
  LDLIBS += -lsocket -lnsl -lrt -lresolv
  INSTALL = ginstall
@ENDIF
@IFEQ $(SYSTEM) darwin
  LDLIBS += -ldl
  CORELDFLAGS = -dynamic -bind_at_load -L. $(LDFLAGS)
  PICLDFLAGS = -fPIC -shared -twolevel_namespace -undefined dynamic_lookup $(LDFLAGS)
@ENDIF

@IFNDEF D
  D=0
@ENDIF

DBGOK=0
@IFEQ $(D) 0
  CORECXXFLAGS += -fno-rtti -O2
@IFEQ $(COMPILER) GCC
    CORECXXFLAGS += -g1
@ENDIF
  HEADER = std-header
  DBGOK=1
@ENDIF
@IFEQ $(D) 1
  CORECXXFLAGS += -O0 -g3 -Werror -DINSPIRCD_ENABLE_RTTI
  HEADER = debug-header
  DBGOK=1
@ENDIF
@IFEQ $(D) 2
  CORECXXFLAGS += -fno-rtti -O2 -g3
  HEADER = debug-header
  DBGOK=1
@ENDIF
FOOTER = finishmessage

@TARGET GNU_MAKE MAKEFLAGS += --no-print-directory

@TARGET GNU_MAKE SOURCEPATH = $(shell /bin/pwd)
@TARGET BSD_MAKE SOURCEPATH != /bin/pwd

@IFDEF V
  VERBOSE = -v
@ELSE
  @TARGET GNU_MAKE MAKEFLAGS += --silent
  @TARGET BSD_MAKE MAKE += -s
  VERBOSE =
@ENDIF

@IFDEF PURE_STATIC
  CORECXXFLAGS += -DPURE_STATIC
@ENDIF

# Add the users CXXFLAGS to the base ones to allow them to override
# things like -Wfatal-errors if they wish to.
CORECXXFLAGS += $(CXXFLAGS)

@DO_EXPORT CXX CORECXXFLAGS LDLIBS PICLDFLAGS VERBOSE SOCKETENGINE CORELDFLAGS
@DO_EXPORT SOURCEPATH BUILDPATH PURE_STATIC

# Default target
TARGET = all

@IFDEF M
    HEADER = mod-header
    FOOTER = mod-footer
    @TARGET BSD_MAKE TARGET = modules/${M:S/.so$//}.so
    @TARGET GNU_MAKE TARGET = modules/$(M:.so=).so
@ENDIF

@IFDEF T
    HEADER =
    FOOTER = target
    TARGET = $(T)
@ENDIF

@IFEQ $(DBGOK) 0
  HEADER = unknown-debug-level
@ENDIF

all: $(FOOTER)

target: $(HEADER)
	$(MAKEENV) perl make/calcdep.pl
	cd $(BUILDPATH); $(MAKEENV) $(MAKE) -f real.mk $(TARGET)

debug:
	@${MAKE} D=1 all

debug-header:
	@echo "*************************************"
	@echo "*    BUILDING WITH DEBUG SYMBOLS    *"
	@echo "*                                   *"
	@echo "*   This will take a *long* time.   *"
	@echo "*  Please be aware that this build  *"
	@echo "*  will consume a very large amount *"
	@echo "*  of disk space (~350MB), and may  *"
	@echo "*  run slower. Use the debug build  *"
	@echo "*  for module development or if you *"
	@echo "*    are experiencing problems.     *"
	@echo "*                                   *"
	@echo "*************************************"

mod-header:
@IFDEF PURE_STATIC
	@echo 'Cannot build single modules in pure-static build'
	@exit 1
@ENDIF
	@echo 'Building single module:'

mod-footer: target
	@echo 'To install, copy $(BUILDPATH)/$(TARGET) to $(MODPATH)'
	@echo 'Or, run "make install"'

std-header:
	@echo "*************************************"
	@echo "*       BUILDING INSPIRCD           *"
	@echo "*                                   *"
	@echo "*   This will take a *long* time.   *"
	@echo "*     Why not read our wiki at      *"
	@echo "*     http://wiki.inspircd.org      *"
	@echo "*  while you wait for make to run?  *"
	@echo "*************************************"

finishmessage: target
	@echo ""
	@echo "*************************************"
	@echo "*        BUILD COMPLETE!            *"
	@echo "*                                   *"
	@echo "*   To install InspIRCd, type:      *"
	@echo "*         make install              *"
	@echo "*************************************"

install: target
	@if [ "$(INSTUID)" = 0 -o "$(INSTUID)" = root ]; then \
		echo ""; \
		echo "Error: You must specify a non-root UID for the server"; \
		echo ""; \
		echo "If you are making a package, please specify using ./configure --uid"; \
		echo "Otherwise, rerun using 'make INSTUID=irc install', where 'irc' is the user"; \
		echo "who will be running the ircd. You will also need to modify the start script."; \
		echo ""; \
		exit 1; \
	fi
	@-$(INSTALL) -d -o $(INSTUID) -m $(INSTMODE_DIR) $(BASE)
	@-$(INSTALL) -d -o $(INSTUID) -m $(INSTMODE_DIR) $(BASE)/data
	@-$(INSTALL) -d -o $(INSTUID) -m $(INSTMODE_DIR) $(BASE)/logs
	@-$(INSTALL) -d -m $(INSTMODE_DIR) $(BINPATH)
	@-$(INSTALL) -d -m $(INSTMODE_DIR) $(CONPATH)/examples/aliases
	@-$(INSTALL) -d -m $(INSTMODE_DIR) $(CONPATH)/examples/modules
	@-$(INSTALL) -d -m $(INSTMODE_DIR) $(MANPATH)
	@-$(INSTALL) -d -m $(INSTMODE_DIR) $(MODPATH)
	[ $(BUILDPATH)/bin/ -ef $(BINPATH) ] || $(INSTALL) -m $(INSTMODE_BIN) $(BUILDPATH)/bin/inspircd $(BINPATH)
@IFNDEF PURE_STATIC
	[ $(BUILDPATH)/modules/ -ef $(MODPATH) ] || $(INSTALL) -m $(INSTMODE_LIB) $(BUILDPATH)/modules/*.so $(MODPATH)
@ENDIF
	-$(INSTALL) -m $(INSTMODE_BIN) inspircd $(BASE) 2>/dev/null
	-$(INSTALL) -m $(INSTMODE_LIB) .gdbargs $(BASE)/.gdbargs 2>/dev/null
@IFEQ $(SYSTEM) darwin
	-$(INSTALL) -m $(INSTMODE_BIN) org.inspircd.plist $(BASE) 2>/dev/null
@ENDIF
@IFEQ $(SYSTEM) linux
	-$(INSTALL) -m $(INSTMODE_LIB) inspircd.service $(BASE) 2>/dev/null
@ENDIF
	-$(INSTALL) -m $(INSTMODE_LIB) inspircd.1 $(MANPATH) 2>/dev/null
	-$(INSTALL) -m $(INSTMODE_LIB) inspircd-genssl.1 $(MANPATH) 2>/dev/null
	-$(INSTALL) -m $(INSTMODE_BIN) tools/genssl $(BINPATH)/inspircd-genssl 2>/dev/null
	-$(INSTALL) -m $(INSTMODE_LIB) docs/conf/*.example $(CONPATH)/examples
	-$(INSTALL) -m $(INSTMODE_LIB) *.pem $(CONPATH) 2>/dev/null
	-$(INSTALL) -m $(INSTMODE_LIB) docs/conf/aliases/*.example $(CONPATH)/examples/aliases
	-$(INSTALL) -m $(INSTMODE_LIB) docs/conf/modules/*.example $(CONPATH)/examples/modules
	@echo ""
	@echo "*************************************"
	@echo "*        INSTALL COMPLETE!          *"
	@echo "*************************************"
	@echo 'Paths:'
	@echo '  Base install:' $(BASE)
	@echo '  Configuration:' $(CONPATH)
	@echo '  Binaries:' $(BINPATH)
	@echo '  Modules:' $(MODPATH)
	@echo '  Data:' $(DATPATH)
	@echo 'To start the ircd, run:' $(BASE)/inspircd start
	@echo 'Remember to create your config file:' $(CONPATH)/inspircd.conf
	@echo 'Examples are available at:' $(CONPATH)/examples/

GNUmakefile BSDmakefile: make/template/main.mk src/version.sh configure @CONFIGURE_CACHE_FILE@
	./configure --update
@TARGET BSD_MAKE .MAKEFILEDEPS: BSDmakefile

clean:
	@echo Cleaning...
	-rm -f $(BUILDPATH)/bin/inspircd $(BUILDPATH)/include $(BUILDPATH)/real.mk
	-rm -rf $(BUILDPATH)/obj $(BUILDPATH)/modules
	@-rmdir $(BUILDPATH)/bin 2>/dev/null
	@-rmdir $(BUILDPATH) 2>/dev/null
	@echo Completed.

deinstall:
	-rm -f $(BINPATH)/inspircd
	-rm -rf $(CONPATH)/examples
	-rm -f $(MANPATH)/inspircd.1
	-rm -f $(MANPATH)/inspircd-genssl.1
	-rm -f $(MODPATH)/*.so
	-rm -f $(BASE)/.gdbargs
	-rm -f $(BASE)/inspircd.service
	-rm -f $(BASE)/org.inspircd.plist

configureclean:
	rm -f BSDmakefile
	rm -f GNUmakefile
	rm -f include/config.h
	rm -f inspircd
	rm -f inspircd.1
	rm -f inspircd-genssl.1
	-rm -f inspircd.service
	-rm -f org.inspircd.plist
	-rm -f @CONFIGURE_CACHE_FILE@

distclean: clean configureclean
	-rm -rf $(SOURCEPATH)/run
	find $(SOURCEPATH)/src/modules -type l | xargs rm -f

help:
	@echo 'InspIRCd Makefile'
	@echo ''
	@echo 'Use: ${MAKE} [flags] [targets]'
	@echo ''
	@echo 'Flags:'
	@echo ' V=1       Show the full command being executed instead of "BUILD: dns.cpp"'
	@echo ' D=1       Enable debug build, for module development or crash tracing'
	@echo ' D=2       Enable debug build with optimizations, for detailed backtraces'
	@echo ' DESTDIR=  Specify a destination root directory (for tarball creation)'
	@echo ' -j <N>    Run a parallel build using N jobs'
	@echo ''
	@echo 'Targets:'
	@echo ' all       Complete build of InspIRCd, without installing (default)'
	@echo ' install   Build and install InspIRCd to the directory chosen in ./configure'
	@echo '           Currently installs to ${BASE}'
	@echo ' debug     Compile a debug build. Equivalent to "make D=1 all"'
	@echo ''
	@echo ' M=m_foo   Builds a single module (cmd_foo also works here)'
	@echo ' T=target  Builds a user-specified target, such as "inspircd" or "modules"'
	@echo '           Other targets are specified by their path in the build directory'
	@echo '           Multiple targets may be separated by a space'
	@echo ''
	@echo ' clean     Cleans object files produced by the compile'
	@echo ' distclean Cleans all generated files (build, configure, run, etc)'
	@echo ' deinstall Removes the files created by "make install"'
	@echo

.PHONY: all target debug debug-header mod-header mod-footer std-header finishmessage install clean deinstall configureclean help

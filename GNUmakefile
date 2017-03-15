PREFIX = /usr/local
BUILDDIR = build
CXX = c++
CPPFLAGS =
CXXFLAGS =
AR = ar
ARFLAGS =
DEBUG =

-include .makesettings

override define settings :=
PREFIX = $(PREFIX)
BUILDDIR = $(BUILDDIR)
CXX = $(CXX)
CPPFLAGS = $(CPPFLAGS)
CXXFLAGS = $(CXXFLAGS)
AR = $(AR)
ARFLAGS = $(ARFLAGS)
DEBUG = $(DEBUG)
endef

$(file >.makesettings,$(settings))

override define prependflags :=
override CPPFLAGS = -iquote include -iquote src -MMD -MT $$@ -MF $$(@:%.o=%.d) $(CPPFLAGS)
override CXXFLAGS = -std=c++14 -pedantic -Wall -Wextra -Werror $(CXXFLAGS)
override ARFLAGS = rc $(ARFLAGS)
endef

$(eval $(call prependflags))

override libobjs := $(patsubst %.cc,$(BUILDDIR)/%.o,$(wildcard src/*.cc))
override testobjs := $(libobjs) $(patsubst %.cc,$(BUILDDIR)/%.o,$(wildcard test/*.cc))

override cmds := help build test install uninstall tag clean
.PHONY: $(cmds)


help:
	@echo usage: $(MAKE) { $(foreach cmd,$(cmds),$(cmd) \|) ... }


build: $(BUILDDIR)/libsiren-http.a


test: $(BUILDDIR)/siren-http-test
	$(DEBUG) $(BUILDDIR)/siren-http-test


install: build
	mkdir --parents $(PREFIX)/lib
	cp --no-target-directory $(BUILDDIR)/libsiren-http.a $(PREFIX)/lib/libsiren-http.a
	mkdir --parents $(PREFIX)/include
	cp --no-target-directory --recursive include $(PREFIX)/include/siren.http


uninstall:
	rm --force $(PREFIX)/lib/libsiren-http.a
	rmdir --parents --ignore-fail-on-non-empty $(PREFIX)/lib
	rm --force --recursive $(PREFIX)/include/siren.http
	rmdir --parents --ignore-fail-on-non-empty $(PREFIX)/include


tag:
	GTAGSFORCECPP= gtags --incremental


clean:
	rm --force --recursive $(BUILDDIR)


$(BUILDDIR)/libsiren-http.a: $(libobjs)
	@mkdir --parents $(@D)
	$(AR) $(ARFLAGS) $@ $^


ifneq ($(filter $(BUILDDIR)/libsiren-http.a build install,$(MAKECMDGOALS)),)
-include $(libobjs:%.o=%.d)
endif


$(BUILDDIR)/siren-http-test: $(testobjs)
	@mkdir --parents $(@D)
	$(CXX) -o $@ $^ -lsiren -lpthread


ifneq ($(filter $(BUILDDIR)/siren-http-test test,$(MAKECMDGOALS)),)
-include $(testobjs:%.o=%.d)
endif


$(BUILDDIR)/%.o: %.cc
	@mkdir --parents $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

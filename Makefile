VERBOSE = @

SRCFOLDER = src
BUILDDIR ?= .build
BINPREFIX = elfo-

CPPLINT ?= cpplint
TIDY ?= clang-tidy
TIDYCONFIG ?= .clang-tidy

CXXFLAGS ?= -Og -g -static-pie
CXXFLAGS += -I include/
CXXFLAGS += -Wall -Wextra -Wno-switch -Wno-unused-variable -Wno-comment

INSTALLDIR ?= $(HOME)/.local/bin

SOURCES := $(wildcard $(SRCFOLDER)/*.cpp)
TARGETS := $(addprefix $(BINPREFIX),$(notdir $(SOURCES:%.cpp=%)))
DEPFILES := $(addprefix $(BUILDDIR)/,$(addsuffix .d,$(TARGETS)))
LDFLAGS :=

GENFILES := $(addprefix $(SRCFOLDER)/,_str_const.hpp _str_ident.hpp)

TESTFOLDER = test
TESTTARGET = $(TESTFOLDER)/h2g2
TESTS := $(patsubst $(TESTFOLDER)/%.stdout,test-%,$(wildcard $(TESTFOLDER)/*.stdout))

ifdef DLH
CXXFLAGS += -std=c++17 -I $(DLH)/legacy -I $(DLH)/include -L $(DLH) -DUSE_DLH
CXXFLAGS += -fno-exceptions -fno-rtti -fno-use-cxa-atexit -no-pie
CXXFLAGS += -nostdlib -nostdinc
LDFLAGS += -ldlh
endif

all: $(TARGETS) $(TESTS)

test: $(TESTS)

test-%: $(TESTFOLDER)/%.stdout $(BINPREFIX)% $(TESTTARGET)
	@echo "Test		$*"
	@./$(BINPREFIX)$* $(TESTTARGET) | diff -w $< -

$(BUILDDIR)/%.d: $(SRCFOLDER)/%.cpp $(GENFILES) $(BUILDDIR) $(MAKEFILE_LIST)
	@echo "DEP		$<"
	$(VERBOSE) $(CXX) $(CXXFLAGS) -MM -MP -MT $* -MF $@ $<

$(BINPREFIX)%: $(SRCFOLDER)/%.cpp $(GENFILES) $(MAKEFILE_LIST)
	@echo "CXX		$@"
	$(VERBOSE) $(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

define include_str_template =
$$(SRCFOLDER)/_str_$(notdir $(1)): include/elfo/$(1) tools/enum2str.py $$(MAKEFILE_LIST)
	@echo "GEN		$$@"
	$$(VERBOSE) $$(CXX) -fpreprocessed -dD -E $$< 2>/dev/null | tools/enum2str.py elfo/$(1) $(2) > $$@

clean::
	$$(VERBOSE) rm -f $$(SRCFOLDER)/_str_$(notdir $(1))
endef

$(eval $(call include_str_template,elf_def/const.hpp,ELF_Def Constants))
$(eval $(call include_str_template,elf_def/ident.hpp,ELF_Def Identification))

lint::
	@if $(CPPLINT) --quiet --recursive . ; then \
		echo "Congratulations, coding style obeyed!" ; \
	else \
		echo "Coding style violated -- see CPPLINT.cfg for details" ; \
		exit 1 ; \
	fi

tidy:: $(TIDYCONFIG)
	$(VERBOSE) $(TIDY) --config-file=$< $(filter-out $(GENFILES),$(SOURCES)) -- -stdlib=libc++  $(CXXFLAGS)

install: $(TARGETS)
	$(VERBOSE) install -Dm755 $^ $(INSTALLDIR)
	$(VERBOSE) if ! echo "$(PATH)" | grep "$(INSTALLDIR)" >/dev/null 2>&1 ; then \
		echo "You have to add '$(INSTALLDIR)' to PATH!" ; \
	fi

clean::
	$(VERBOSE) rm -f $(DEPFILES) $(GENFILES)
	$(VERBOSE) test -d $(BUILDDIR) && rmdir $(BUILDDIR) || true

mrproper:: clean
	$(VERBOSE) rm -f $(TARGETS)

$(BUILDDIR): ; @mkdir -p $@

$(DEPFILES):

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILES)
endif

.PHONY: all test clean lint tidy install clean mrproper

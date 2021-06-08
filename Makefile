DEPDIR := .deps
CXXFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$@.d -Og -g -I include/
CXXFLAGS += -Wall -Wextra -Wno-switch -Wno-unused-variable -Wno-comment
SOURCES := $(wildcard src/*.cpp)
TARGETS := $(notdir $(SOURCES:%.cpp=%))
DEPFILES := $(addprefix $(DEPDIR)/,$(addsuffix .d,$(TARGETS)))
LDFLAGS :=

ifdef DLH
CXXFLAGS += -std=c++17 -I $(DLH)/include -L $(DLH) -DUSE_DLH
CXXFLAGS += -fno-exceptions -fno-rtti -fno-use-cxa-atexit -no-pie
CXXFLAGS += -nostdlib -nostdinc
LDFLAGS += -ldlh
endif

all: $(TARGETS)

define include_str_template =
src/_str_$(notdir $(1)): include/$(1) tools/enum2str.py Makefile
	$$(CXX) -fpreprocessed -dD -E $$< 2>/dev/null | tools/enum2str.py $(1) $(2) > $$@

clean::
	rm -f src/_str_$(notdir $(1))
endef

$(eval $(call include_str_template,elf_def/const.hpp,ELF_Def Constants))
$(eval $(call include_str_template,elf_def/ident.hpp,ELF_Def Identification))

%: src/%.cpp src/_str_const.hpp src/_str_ident.hpp Makefile | $(DEPDIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(DEPDIR): ; @mkdir -p $@

$(DEPFILES):

clean::
	rm -f $(DEPFILES)
	rmdir $(DEPDIR) || true

include $(wildcard $(DEPFILES))

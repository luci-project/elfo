
INCLUDEDIR = include/
CXXFLAGS = -Og -g -I include/

all: dump lookup

src/_str_const.hpp: include/elf_def/const.hpp tools/enum2str.py Makefile
	$(CXX) -fpreprocessed -dD -E $< 2>/dev/null | python3 tools/enum2str.py elf_def/const.hpp ELF_Def Constants > $@

src/_str_ident.hpp: include/elf_def/ident.hpp tools/enum2str.py Makefile
	$(CXX) -fpreprocessed -dD -E $< 2>/dev/null | python3 tools/enum2str.py elf_def/ident.hpp ELF_Def Identification > $@

dump: src/dump.cpp src/_str_const.hpp src/_str_ident.hpp Makefile
	$(CXX) $(CXXFLAGS) -o $@ $<

lookup: src/lookup.cpp Makefile
	$(CXX) $(CXXFLAGS) -o $@ $<

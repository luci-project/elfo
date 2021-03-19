
INCLUDEDIR = include/
CXXFLAGS = -I include/

all: dump

src/_str_const.hpp: include/elf_def/const.hpp enum2str.py Makefile
	$(CXX) -fpreprocessed -dD -E $< 2>/dev/null | python3 tools/enum2str.py elf_def/const.hpp ELF_Def Constants  > $@

dump: src/dump.cpp src/_str_const.hpp  Makefile
	$(CXX) $(CXXFLAGS) -o $@ $<

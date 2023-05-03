Hi, I'm Elfo!
=============

A really lightweight, optimistic & naive C++ (header-only) parser for the [Executable and Linking Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format), supporting common GNU/[Linux](https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/elf-generic.html) extensions.

This library was written for a [small dynamic linker & loader](https://gitlab.cs.fau.de/luci-project/luci), hence it directly accesses the memory mapped file without requiring dynamic memory allocation.
However, it is still designed with academic purposes in mind and therefore not optimized for best performance.

Unless you are happy with those limitations, you should better take look at more mature projects like [ELFIO](https://github.com/serge1/ELFIO).


Examples
--------

The `src` directory contains a few example programs, which can be built using

    make

(a current C++ compiler and [Python 3](https://www.python.org/) with [PyParsing](https://pypi.org/project/pyparsing/) are required for the build process)


### Dump

Displays ELF file information, similar to [GNU Binutils](https://www.gnu.org/software/binutils/) `readelf -a -W`.

    ./elfo-dump test/h2g2

`h2g2` is a prebuilt test binary (see [h2g2,cpp](test/h2g2.cpp)), the output should be identical to [dump.stdout](test/dump.stdout).


### Dynamic-Dump

Displays ELF header and all information accessible via *DYNAMIC* section.

    ./elfo-dynamic-dump test/h2g2

The output should be identical to [dynamic-dump.stdout](test/dynamic-dump.stdout).


### vDSO-Dump

Dump the contents of the virtual dynamic shared object (residing in memory) into a local file:

    ./elfo-vdso-dump /tmp/vdso

will dump the contents into `/tmp/vdso` -- this should create a valid ELF file, which can be insepected by `elfo-dump`.

### Lookup

Find a symbol in the dynamic section of an ELF file (using hash, if possible).

    ./elfo-lookup test/h2g2

The output should be identical to [lookup.stdout](test/lookup.stdout).


### setInterp

Change the interpreter string (to a path with less or equal length):

    g++ -o foo test/h2g2
    ./elfo-setinterp foo /opt/luci/ld-luci.so 

will change the default interpreter `/lib64/ld-linux-x86-64.so.2` (on Debian) to `/opt/luci/ld-luci.so`


Author & License
----------------

*Elfo* is part of the *Luci*-project, which is being developed by [Bernhard Heinloth](https://sys.cs.fau.de/person/heinloth) of the [Department of Computer Science 4](https://sys.cs.fau.de/) at [Friedrich-Alexander-Universität Erlangen-Nürnberg](https://www.fau.eu/) and is available under the [GNU Affero General Public License, Version 3 (AGPL v3)](LICENSE.md).

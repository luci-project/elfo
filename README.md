Hi, I'm Elfo!
=============

A really lightweight, optimistic & naive C++ (header-only) parser for the [Executable and Linking Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format), supporting common GNU/[Linux](https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/elf-generic.html) extensions.

This library was written for a small dynamic linker & loader, hence it directly accesses the memory mapped file without requiring dynamic memory allocation.
However, it is still designed with academic purposes in mind and therefore not optimized for best performance.

Unless you are happy with those limitations, you should better take look at more mature projects like [ELFIO](https://github.com/serge1/ELFIO).


Examples
--------

The `src` directory contains a few example programs, which can be built using

    make

(a current C++ compiler and [Python 3](https://www.python.org/) are required for the build process).

### Dump

Displays ELF file information, similar to [GNU Binutils](https://www.gnu.org/software/binutils/) `readelf -a -W`.

### Dynamic-Dump

Displays ELF header and all information accessible via *DYNAMIC* section.

### vDSO-Dump

Dump the contents of the virtual dynamic shared object (residing in memory) into a local file.

### Lookup

Find a symbol in the dynamic section of an ELF file (using hash, if possible).

### setInterp

Change the interpreter string (to a path with less or equal length)


Author & License
----------------

*Elfo* is part of the *Luci*-project, which is being developed by [Bernhard Heinloth](https://sys.cs.fau.de/person/heinloth) of the [Department of Computer Science 4](https://sys.cs.fau.de/) at [Friedrich-Alexander-Universität Erlangen-Nürnberg](https://www.fau.eu/) and is available under the [GNU Affero General Public License, Version 3 (AGPL v3)](LICENSE.md).


Name
----

Its name origins from the [Disenchantment](https://en.wikipedia.org/wiki/Disenchantment_(TV_series)) [character](https://disenchantment.fandom.com/wiki/Elfo), obviously.
Thanks for all the fun, [Matt](https://en.wikipedia.org/wiki/Matt_Groening)!

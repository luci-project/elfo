Hi, I'm Elfo!
=============

A really lightweight, optimistic & naive C++ (header-only) parser for the [Executable and Linking Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format), supporting common GNU/[Linux](https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/elf-generic.html) extensions.

This library was written for a small dynamic linker & loader, hence it directly accesses the memory mapped file without requiring dynamic memory allocation.
However, it is still designed with academic purposes in mind and therefore not optimized for best performance.

Unless you are happy with those limitations, you should better take look at more mature projects like [ELFIO](https://github.com/serge1/ELFIO).


Examples
--------

The `src` directory contains two example programs, which can be built using

    make

(a current C++ compiler and [Python 3](https://www.python.org/) are required for the build process).

### Dump

Displays ELF file information, similar to [GNU Binutils](https://www.gnu.org/software/binutils/) `readelf -a -W`.

### Lookup

Find a symbol in the dynamic section of an ELF file (using hash, if possible).


Name
----

Its name origins from the [Disenchantment](https://en.wikipedia.org/wiki/Disenchantment_(TV_series)) [character](https://disenchantment.fandom.com/wiki/Elfo), obviously.
Thanks for all the fun, [Matt](https://en.wikipedia.org/wiki/Matt_Groening)!

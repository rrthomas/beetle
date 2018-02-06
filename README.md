# Beetle

by Reuben Thomas <rrt@sc3d.org>  
https://github.com/rrthomas/beetle  

Beetle is a virtual machine designed for the Forth language. It uses a
byte-stream code designed for efficient execution which is binary portable
between implementations. It has been implemented in C (for POSIX systems)
and hand-optimised assembler (for ARM). The C implementation should run on
any POSIX system; the assembler version runs the supplied Forth compiler at
up to half the speed of the corresponding native code compiler and generates
more compact code. Beetle is designed to be embedded in other programs; a
simple debugger has been written to demonstrate this ability. In the C
implementation, all memory references are bounds checked. A standard I/O
library is implemented; access to native code routines is also possible,
allowing Forth and C programs to call each other.

This package comprises the definition of the Beetle virtual machine, a
virtual machine specialised for the execution of Forth, an
implementation in ISO C99 using POSIX APIs, and a full ANSI Forth
compiler, pForth. Detailed documentation is in the doc directory;
installation instructions follow.

The package is distributed under the GNU Public License version 3, or,
at your option, any later version.

THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
RISK.


## Installation and compatibility

Beetle should work on any POSIX-1.2001-compatible system that uses
two’s complement arithmetic.

Beetle has been tested on GNU/Linux with GNU C.

Previous releases were known to work on Acorn RISC OS 3, Digital UNIX
V3.2, UNIX System V Release 4.0, ULTRIX 4.3, NetBSD 1.2, MSDOS 6, and
Atari TOS 1.4.

Reports on compatibility, whether positive or negative, are welcomed.

To build Beetle from a release tarball, run

`./configure && make && make check`

If building from a Git repository, first run

`./bootstrap`

For the bibliographies in the documentation to be built correctly, GNU Make
should be used.


## Documentation

I developed Beetle for my BA dissertation project. _(I used to refer to it as a “virtual processor”; I now use the now-standard term “virtual machine”.)_ The following documents include extra material not covered in Beetle’s documentation:

* _[An Introduction to the Beetle Forth Virtual Processor](doc/intro.pdf)_  
An introduction to the system; this is the best paper to read first. It was published in ACM SIGPLAN Notices February 1997.
* _[The Beetle Forth Virtual Machine](doc/beetle.pdf)_  
The design of the Beetle Forth virtual machine is described. This is an implementor's guide, but it's probably also useful to programmers.
* _[An implementation of the Beetle virtual machine in ANSI C](doc/cbeetle.pdf)_  
A portable ANSI C implementation of Beetle is described, with instructions for porting, compiling and running it.
* _[A simple user-interface for the Beetle virtual machine](doc/uiface.pdf)_  
A user guide for a simple debugger shell for Beetle.
* _[Beetle and pForth: a Forth virtual machine and compiler](https://rrt.sc3d.org/Software/Beetle/dissertation/report/badiss.pdf)_  
My BA dissertation containing all the papers mentioned above, as well as a description of the project that produced them.
* _[Tradeoffs in the implementation of the Beetle virtual machine](doc/tradeoffs.pdf)_  
A hand-coded implementation of Beetle is described, and compared to the C version.
* _[Encoding literals in a portable byte-stream interpreter](doc/litencode.pdf)_  
Various methods of encoding literal numbers in a byte stream are compared.


## pForth

[pForth](https://github.com/rrthomas/pforth) is an ANSI Forth compiler that
targets Beetle.


## Running Beetle images

On Linux systems with binfmt_misc, it is possible to make Beetle object
modules directly runnable.

For example, on Debian or Ubuntu, run the command:

```
sudo update-binfmts --install beetle `which beetle` --magic "\x42\x45\x45\x54\x4c\x45\x00"
```

A magic file for the file(1) command is also provided: beetle.magic.
This file should be part of file >= 5.33.


## Hand-written ARM assembler version

`ARMbeetle.bas` contains a hand-written ARM assembler version of Beetle,
written in the BBC BASIC assembler (for RISC OS). It produces AOF
format objects equivalent to those produced by run.c and step.c, which
can be linked in their place.


## Bugs and comments

Please send bug reports (preferably as [GitHub issues](https://github.com/rrthomas/beetle/issues)) and
comments. I‘m especially interested to know of portability bugs.

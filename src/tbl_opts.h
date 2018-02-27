/* Table of command-line options

   Copyright (c) 2009-2018 Reuben Thomas
 */

/*
 * D: documentation line
 * O: Option
 * A: Action
 *
 * D(text)
 * O(long name, short name ('\0' for none), argument, argument docstring, docstring)
 * A(argument, docstring)
 */

#define xstr(s) #s
#define str(s) xstr(s)
#define MEMORY_MESSAGE "set MEMORY to the given NUMBER of cells\n" \
  "                        0 < NUMBER <= " str(MAX_MEMORY) " [default " str(DEFAULT_MEMORY) "]"
O("memory", 'm', required_argument, "NUMBER", MEMORY_MESSAGE)
O("help", '\0', no_argument, "", "display this help message and exit")
O("version", '\0', no_argument, "", "display version information and exit")
A("FILE", "load and run object FILE")

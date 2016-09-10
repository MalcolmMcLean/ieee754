# ieee754
Ieee754 floating point routines, load and save ieee 754 even on non-ieee 754 hardware. 

You often need to load or save floating point numbers in binary format to disk.
Because not all hardware supports IEEE 754, ANSI C does not specify which format 
floating points shall be stored in internally. So simply dumping a binary image
to disk is not guaranteed to work if the file must be opened by a second program.

These routines read and write IEEE  754 number portably, regardless of the computer's
internal format. If the format is not compatible with IEEE 754 they should save and
load the nearest possible representation.

You should use them anytime you need to load or save binary floating point numbers 
and the code needs to be platform-independent.

A related issue is reading binary integers portably. It is slightly tricker than it first appears and the simple shift and add algorithm will fail if CHAR_BIT is not 8 or representation is not 2's complement. It can also generate a signed arithmetic overflow trap if not careful. Code has been added to address this question. Any C or C++ code that 
reads integers in binary format and needs to be portable should use these routines.



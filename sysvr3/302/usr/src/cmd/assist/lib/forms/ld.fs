00000ld
00701040130NAME(S) OF INPUT FILE(S):
01703040001EXECUTABLE OUTPUT OBJECT FILE (a.out is default) [-o]:
02704040001PRODUCE A MAP OR LISTING OF INPUT/OUTPUT SECTIONS [-m] (y/n):
03705040001OUTPUT FILE WILL BECOME INPUT FILE IN SUBSEQUENT "ld" RUNS [-r] (y/n):
04706040001STRIP LINE NUMBER AND SYMBOL TABLE INFO FROM OBJECT FILE [-s] (y/n):
05707040001TURN OFF WARNING ABOUT MULTIPLY-DEFINED SYMBOLS [-t] (y/n):
06708040001DO NOT PRESERVE LOCAL SYMBOLS IN THE OUTPUT SYMBOL TABLE [-x] (y/n):
07709040001DO NOT BIND ANYTHING TO ADDRESS ZERO [-z] (y/n):
08710040001OUTPUT  MESSAGE FOR MULTIPLY-DEFINED EXTERNAL DEFINITIONS [-M] (y/n):
09711040001REORDER TEXT AND DATA INFO SECTIONS IN THE CORE IMAGE [-N] (y/n):
10712040001OUTPUT INFO ABOUT THE VERSION OF "ld" BEING USED [-V] (y/n):
11713040001SET DEFAULT ENTRY POINT ADDRESS FOR THE OUTPUT FILE [-e]:
12714040001SET DEFAULT FILL PATTERN FOR "HOLES" [-f]:
13715040001REPLACE THE DEFAULT LIBRARY DIRECTORY LIBDIR  [-YL]:
14716040001REPLACE THE DEFAULT LIBRARY DIRECTORY LLIBDIR [-YU]:
15717040030SEARCH LIBRARIE(S), libx.a.  [-l] (ENTER x):
16718040030SEARCH LIBRARIE(S), libx.a., IN A SPECIFIED DIRECTORY [-L]:
17719040030MAKE AN UNDEFINED ENTRY IN THE SYMBOL TABLE [-u]:
18720040001IDENTIFY THE a.out FILE WITH A SPECIFIED VERSION NUMBER [-VS]:
19722040030ADDITIONS TO THE COMMAND LINE:

00110
01010-o a.out
02010
03010
04010
05010
06010
07010
08010
09010
10010
11130
12130
13130
14130
15190
16180
17120
18130
19992


The "ld" command combines several object files into one, performs
relocation, resolves external symbols, and supports symbol table
information for symbolic debugging.  In the simplest case, the 
names of several object programs are given, and "ld" combines the
objects, producing an object module that can either be executed or
used as input for a subsequent "ld" run.  The output of "ld" is 
left in a.out.  By default this file is executable if no errors 
occured during the load.  If any input file is not an object file,
"ld" assumes it is either an archive library or a text file containing
link editor directives.

00The entry here should be one or more object
code files or archived files (.a files).
01The executable object file can be given any name.  If you
leave this field empty, the executable output file 
produced will be named "a.out".   [-o]
02Produce a map or listing of the input output sections
on the standard output.   [-m]
03Retain relocation entries in the output object file.  
Relocation entries must be saved if the output file is 
to become an input file in a subsequent ld run.  The 
link editor will not complain about unresolved references,
and the output file will not be executable.  [-r]
04Strip line number entries and symbol table information
from the output object file.   [-s]
05Turn off the warning about multiply-defined symbols
that are not the same size.   [-t]
06Do not preserve local symbols in the output symbol table;  enter
external and static symbols only.  This option saves some space 
in the output file.   [-x]
07Do not bind anything to address zero.  This option will allow 
run-time detection of null pointers.  [-z]
08Output a message for each multiply-defined external
definition.   [-M]
09Put the text section at the beginning of the text segment rather
than after all header information, and put the data section immediately
following text in the core image.   [-N]
10Output a message giving information about the version
of ld being used.   [-V]
11Set the default entry point address for the output file to be 
the symbol that you enter.   [-e]
12Set the default fill pattern for "holes" within an output section
as well as initialized bss sections.  To set the default, enter the
fill pattern (a two-byte constant).   [-f]
13Change the default directory, LIBDIR (usually /lib), used 
for finding libraries.  LIBDIR is the first default directory
which ld searches.  [-YL]
14Change the default directory, LLIBDIR (usually /usr/lib), used
for finding libraries.  LLIBDIR is the second default directory 
which "ld" searches.  If "ld" was built with only one default 
directory and this option is specified a warning is printed and 
this option is ignored.  [-YU]
15Search a library, "libx.a", where "x" is up to nine characters.
A library is searched when its name is encountered, so the
placement of the arguments(s), "x", is significant.  By default,
libraries are located in LIBDIR (usually /lib) or LLIBDIR
(usually /usr/lib.)  

ASSIST does no validation on this field.   [-l]
16Change the algorithm of searching for "libx.a" to look in the
specified directory before looking in LIBDIR and LLIBDIR.  This
option is effective only if you have entered "x" in "libx.a" 
in the previous option of this selection menu.   [-L]
17Enter an undefined symbol in the symbol table.  This is useful for 
loading entirely from a library, since initially the symbol table 
is empty and an unresolved reference is needed to force the loading
of the first routine.  The placement of this option on the "ld" line
is significant;  it must be placed before the library which will 
define the symbol.   [-u]
18Use a specified number as a decimal version stamp identifying the 
"a.out" file that is produced.  The version stamp is stored in the
optional header.   [-VS]
19ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0304
040306
0508
0604
0805

1615

020466n
030575n
040673n
050764n
060873n
070953n
081074n
091170n
101265n

0002
003420001010
0103
0132if test -d $F01;  then echo "$F01 IS A DIRECTORY";fi
0204yYnN
0304yYnN
0305
0404yYnN
0405
0504yYnN
0505
0604yYnN
0605
0704yYnN
0804yYnN
0805
0904yYnN
1004yYnN
1519
1616
1615
1632if test ! -d $F16;  then echo "$F16 IS NOT A DIRECTORY";fi
1732


001110
011010-o 
021011y-mY-mnN
031011y-rY-rnN
041011y-sY-snN
051011y-tY-tnN
061011y-xY-xnN
071011y-zY-znN
081011y-MY-MnN
091011y-NY-NnN
101011y-VY-VnN
111010-e 
121010-f 
131010-YL,
141010-YU,
151110-l
161110-L
171110-u 
181010-VS 
191110




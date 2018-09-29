00000cc
00701040130NAME(S) OF INPUT FILE(S):
01003000030COMPILER-OPTIMIZER-ASSEMBLER "cc" OPTIONS
02704040001PRODUCE AN EXECUTABLE OUTPUT OBJECT FILE (a.out is default) [-o]:
03705040101SUPPRESS LINK EDIT PHASE AND PRODUCE AN OBJECT FILE [-c] (y/n):
04706040101COMPILE FOR "sdb" DEBUGGER [-g] (y/n):
05707040001COMPILE WITH "sdb" DEBUGGER WITHOUT SYMBOL ATTRIBUTE INFO [-ds] (y/n):
06708040001COMPILE WITH "sdb" DEBUGGER WITHOUT DEBUGGING LINE INFO [-dl] (y/n):
07709040001LOAD FLOATING POINT EMULATION CODE [-f] (y/n):
08710040001PRODUCE CODE TO COUNT TIMES EACH ROUTINE IS CALLED [-p] (y/n):
09711040001ALLOW PROFILING WITH "prof" [-pq] (y/n):
10712040001RUN ONLY PREPROCESSOR AND SEND RESULT TO STANDARD OUTPUT [-E] (y/n):
11713040001RUN ONLY PREPROCESSOR AND SEND RESULT TO .i FILES [-P] (y/n):
12714040001GENERATE CODE FOR SINGLE PRECISION ARITHMETIC [-F] (y/n):
13715040001DO COMPILATION PHASE OPTIMIZATION [-O] (y/n):
14716040001DO OPTIMIZATION TUNED FOR SIZE [-K sz] (y/n):
15717040001DO OPTIMIZATION TUNED FOR SPEED [-K sd] (y/n):
16718040001PUT ASSEMBLER-LANGUAGE OUTPUT IN CORRESPONDING .s FILES [-S] (y/n):
17719040001PRINT THE VERSION NUMBERS OF THE COMPILATION TOOLS [-V] (y/n):
18720040030ENTER ARGUMENT(S), argi, TO COMPILATION TOOL(S),c [-W]:
19721040030ENTER LOCATION(S) OF TOOL(S) OR DIRECTORIE(S) [-Y]:
20023000030PREPROCESSOR "cpp" OPTIONS
21724040030REMOVE ANY INITIAL DEFINITION OF name [-u]:
22725040030DEFINE name WITH VALUE def AS IF BY A #define [-D]:
23726040001PASS C-STYLE COMMENTS TO COMPILER [-C] (y/n):
24727040030SEARCH dir FOR #include FILES BEFORE SEARCHING STANDARD LIST [-I]:
25728040030USE dir FOR #include FILES IN PLACE OF STANDARD LIST [-Wp,-Y]:
26729040001PRINT THE PATH NAMES OF INCLUDED FILES [-H] (y/n):
27031000030LINK EDITOR "ld" OPTIONS
28732040001PRODUCE A MAP OR LISTING OF INPUT/OUTPUT SECTIONS [-m] (y/n):
29733040001OUTPUT FILE WILL BECOME INPUT FILE IN SUBSEQUENT "ld" RUNS [-r] (y/n):
30734040001STRIP LINE NUMBER AND SYMBOL TABLE INFO FROM OBJECT FILE [-s] (y/n):
31735040001TURN OFF WARNING ABOUT MULTIPLY-DEFINED SYMBOLS [-t] (y/n):
32736040001DO NOT PRESERVE LOCAL SYMBOLS IN THE OUTPUT SYMBOL TABLE [-x] (y/n):
33737040001DO NOT BIND ANYTHING TO ADDRESS ZERO [-z] (y/n):
34738040001OUTPUT MESSAGE FOR MULTIPLY-DEFINED EXTERNAL DEFINITIONS [-M] (y/n):
35739040001REORDER TEXT AND DATA INFO SECTIONS IN THE CORE IMAGE [-N] (y/n):
36740040001SET DEFAULT ENTRY POINT ADDRESS FOR THE OUTPUT FILE [-e]:
37741040001SET DEFAULT FILL PATTERN FOR "HOLES" [-Wl,-f]:
38742040001REPLACE THE DEFAULT LIBRARY DIRECTORY LIBDIR  [-YL]:
39743040001REPLACE THE DEFAULT LIBRARY DIRECTORY LLIBDIR [-YU]:
40744040030SEARCH LIBRARIES, libx.a. [-l] :
41745040030SEARCH LIBRARIES, libx.a, IN A SPECIFIED DIRECTORY [-L]:
42746040030MAKE AN UNDEFINED ENTRY IN THE SYMBOL TABLE [-U]:
43747040001IDENTIFY THE a.out FILE WITH A SPECIFIED VERSION NUMBER [-VS]:
44749040030ADDITIONS TO THE COMMAND LINE:

00360
02040-o a.out
03050c
04050g
05050
06050
07050
08050
09050
10050
11050
12050
13050
14050
15050
16050
17050
18050
19050
21050
22050
23050
24050
25050
26050
28050
29050
30050
31050
32050
33050
34050
35050
36380
37380
38380
39380
40440
41430
42370
43380
44452


"cc" is the interface for the C Compilation System.  The compilation
tools consist of a preprocessor, compiler, optimizer, assembler, 
and link editor.  The complete ASSIST form for "cc" is divided into 
three categories: "cc", "cpp", and "ld" options.  "cpp" is invoked as 
the first pass of any C compilation by the "cc" command.  The use
of the "cpp" command other than through the "cc" command is not 
suggested.  After "cc" compiles, optimizes, and assembles, "ld"
is them invoked as the link editor.  "ld" can be run as a command
and ASSIST also provides an "ld" form.

00The "cc" command accepts C-source code ( .c files) as input and 
preprocesses, compiles, optimizes, assembles, and link edits these.
Also, the "cc" command accepts assembly-source code ( .s files) as
input and assembles and link edits these.  The "cc" command accepts
preprocessed C-source code ( .i files) as input and compiles, 
optimizes, assembles, and link edits these.  The "cc" command  link
edits all other files and assumes that these are object code files 
( normally suffixed with .o .)
02The executable object file can be given any name.  If you
leave this field empty, the executable output file 
produced will be named "a.out".   [-o]
03This field when invoked, will write the object file(s)
into your directory.  The object files will have ".o"
as a suffix and have the same prefix as your ".c" 
files.   Normally, "cc" links object files, and, in
case only one file is compiled, deletes the ".o" file.  [-c]
04Causes the compiler to generate additional information needed 
for the use of "sdb(1)", a symbolic debugging program.  [-g]
05Don't generate symbol attribute information for the
symbolic debugger.   [-ds]
06Don't generate symbolic debugging line number 
information.   [-dl]
07Cause the editor to load floating point emulation code.
This option is needed only when loading old objects which
use floating point.  Current objects do not require 
floating point emulation code.   [-f]
08Using this option will produce code that counts the number
of times each routine is called;  also, if link editing takes
place, profiled versions of libc.a and libm.a (with -lm option) 
are linked and monitor (3C) is automatically called.  A mon.out
file will then be produced at normal termination of execution
of the object program.  An execution profile can then be 
generated by use of prof(1).   [-p]
09This option will produce profiled code.   [-pq]
10Run only the preprocessor on the named C programs,
and send the result to the standard output.   [-E]
11Run only the preprocessor on the named C programs and
leave the result in corresponding files suffixed ".i."
This option is passed to "cpp(1)".   [-P]
12Cause the compiler to generate code for single precision
arithmetic whenever an expression contains float variables 
and no doubles.   [-F]
13Do compilation phase optimization.  This option will
not have any affect on .s files.  [-O]
14Do optimizations tuned for size.  This option must be
used in conjunction with the -O option.   [-K sz]
15Do optimizations tuned for speed.  This option must be
used in conjunction with the -O option.   [-K sd]
16Compile and do not assemble the named C programs, 
and leave the assembler-language output in cor-
responding files suffixed ".s."   [-S]
17Print the version of the compiler, optimizer, assembler,
and/or link editor that is invoked.   [-V]
18In the form of "c,arg1,arg2,...argi," hand off the
argument[s] "argi" to pass "c" where "c" is p, 0, 
2, a, or l.    [-W]

             p=preprocessor
             0=compiler
             2=optimizer
             a=assembler
             l=link editor

Example:  l,-m,-z  (passes options -m and -z to the
link editor)
19In the form of "c,dirname", specify a new pathname, dirname, for the 
locations of the tools or directories designated in c, where c can be:
---- c -- tool or directory -----------------------------------------
     p    preprocessor
     0    compiler
     2    optimizer                          [-Y]
     a    assembler                           
     l    link editor
     S    directory containing the start-up routines
     I    default include directory searched by cpp(1)
     L    first default library directory searched by ld(1)
     U    second default library directory searched by ld(1)
21Enter "name".  This option will remove any initial definition of 
"name", where "name" is a reserved symbol that is predefined by 
the particular preprocessor.  Current list of "name": unix, dmert,
gcos, ibm, os, tss, interdata, pdp11, u370, u3b, u3b5, u3b2, u3b20d,
vax, RES, RT, lint.  

ASSIST will not do a validation of the "name" that you enter.   [-u]
22For this option, enter "name=def" or just "name".
Define "name" with value "def" as if by a #define.
If no "=def" is given, "name" is defined with value
1.  This option has lower precedence than the "remove
any initial definition of name" [-U] option.  That
is, if the same "name" is used in both options, the
"name" will be undefined regardless of the order of
the options.   [-D]
23By default, "cpp" strips C-style comments.  If this
option is specified, all comments (except those found
on "cpp" directive lines) are passed along.   [-C]
24Change the algorithm for searching for #include files whose names
do not begin with / to look in your specified directory before
looking in the directories on the standard list.  Thus, #include
files whose names are enclosed in "" will be searched for first in
the directory of the file with the  #include line, then in directories
named in this option, and last in directories on a standard list.
For #include files whose names are enclosed in <>, the directory 
of the file with the #include line is not searched.   [-I]
25Use directory dir in place of the standard list of directories
when searching for #include files. The preprocessor, "cpp", uses
the argument -Ydir.  To use this option from the "cc" command, 
the form we use is -Wp,-Ydir.   [Wp,-Y]
26Print, one per line on standard error, the path 
names of included files.   [-H]
28Produce a map or listing of the input output sections
on the standard output.   [-m]
29Retain relocation entries in the output object file.  
Relocation entries must be saved if the output file is 
to become an input file in a subsequent ld run.  The 
link editor will not complain about unresolved references,
and the output file will not be executable.  [-r]
30Strip line number entries and symbol table information
from the output object file.   [-s]
31Turn off the warning about multiply-defined symbols
that are not the same size.   [-t]
32Do not preserve local symbols in the output symbol table;  enter
external and static symbols only.  This option saves some space 
in the output file.   [-x]
33Do not bind anything to address zero.  This option will allow 
run-time detection of null pointers.  [-z]
34Output a message for each multiply-defined external
definition.   [-M]
35Put the text section at the beginning of the text segment rather
than after all header information, and put the data section immediately
following text in the core image.   [-N]
36Set the default entry point address for the output file to be 
the symbol that you enter.   [-e]
37Set the default fill pattern for "holes" within an output section
as well as initialized bss sections.  To set the default, enter the
fill pattern (a two-byte constant). The link editor, "ld", uses
the argument -f fill.  To use this option from the "cc" command, 
the form we use is -Wl,-f fill.   [Wl,-f]
38Change the default directory, LIBDIR (usually /lib), used 
for finding libraries.  LIBDIR is the first default directory
which ld searches.  [-YL]
39Change the default directory, LLIBDIR (usually /usr/lib), used
for finding libraries.  LLIBDIR is the second default directory 
which "ld" searches.  If "ld" was built with only one default 
directory and this option is specified a warning is printed and 
this option is ignored.  [-YU]
40Search a library, "libx.a", where "x" is up to nine characters.
A library is searched when its name is encountered, so the
placement of the arguments(s), "x", is significant.  By default,
libraries are located in LIBDIR (usually /lib) or LLIBDIR
(usually /usr/lib.)  

ASSIST does no validation on this field.   [-l]
41Change the algorithm of searching for "libx.a" to look in the
specified directory before looking in LIBDIR and LLIBDIR.  This
option is effective only if you have entered "x" in "libx.a" 
in the previous option of this selection menu.   [-L]
42Enter an undefined symbol in the symbol table.  This is useful for 
loading entirely from a library, since initially the symbol table 
is empty and an unresolved reference is needed to force the loading
of the first routine.  The placement of this option on the "ld" line
is significant;  it must be placed before the library which will 
define the symbol.   [-U]
43Use a specified number as a decimal version stamp identifying the 
"a.out" file that is produced.  The version stamp is stored in the
optional header.   [-VS]
44ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0203101116
03021011
043013
0530
0630
10110203
11100203
1304
1602
2930
3029050604
3134
3431

0504
0604
1413
1513
4140

030568n
040643n
050775n
060873n
070951n
081067n
091145n
101273n
111366n
121462n
131550n
141650n
151751n
161872n
171967n
232650n
262955n
283266n
293375n
303473n
313564n
323673n
333753n
343873n
353970n

0002
003420001010
0203
0232if test -d $F02;  then echo "$F02 IS A DIRECTORY";fi
0205
0304yYnN
0305
0404yYnN
0419
0405
0504yYnN
0516
0505
0616
0604yYnN
0605
0704yYnN
0804yYnN
0904yYnN
1004yYnN
1005
1104yYnN
1105
1204yYnN
1304yYnN
1319
1305
1404yYnN
1416
1504yYnN
1516
1604yYnN
1605
1704yYnN
1808,p02al
1908,p02alSILU
2304yYnN
2432if test ! -d $F24;  then echo "$F24 IS NOT A DIRECTORY";fi
2532if test ! -d $F25;  then echo "$F25 IS NOT A DIRECTORY";fi
2604yYnN
2804yYnN
2904yYnN
2905
3004yYnN
3005
3104yYnN
3105
3204yYnN
3304yYnN
3404yYnN
3405
3504yYnN
3832if test ! -d $F38;  then echo "$F38 IS NOT A DIRECTORY";fi
3932if test ! -d $F39;  then echo "$F39 IS NOT A DIRECTORY";fi
4019
4116
4132if test ! -d $F41;  then echo "$F41 IS NOT A DIRECTORY";fi
4115


001110
011110
021010-o 
031011y-cY-cnN
041011y-gY-gnN
051011y-dsY-dsnN
061011y-dlY-dlnN
071011y-fY-fnN
081011y-pY-pnN
091011y-pqY-pqnN
101011y-EY-EnN
111011y-PY-PnN
121011y-FY-FnN
131011y-OY-OnN
141011y-K szY-K sznN
151011y-K sdY-K sdnN
161011y-SY-SnN
171011y-VY-VnN
181110-W
191110-Y 
201110
211110-U
221110-D
231011y-CY-CnN
241110-I
251110-Wp,-Y
261011y-HY-HnN
271110
281011y-mY-mnN
291011y-rY-rnN
301011y-sY-snN
311011y-tY-tnN
321011y-xY-xnN
331011y-zY-znN
341011y-MY-MnN
351011y-NY-NnN
361010-e 
371010-Wl,-f 
381010-YL,
391010-YU,
401110-l
411110-L
421110-u 
431010-VS 
441110




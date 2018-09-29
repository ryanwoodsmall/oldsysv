00000cc
00701100130NAME(S) OF INPUT FILE(S):
01003100030PRODUCE AN EXECUTABLE OUTPUT OBJECT FILE
02704120001(a.out is default) [-o]:
03006100130SUPPRESS LINK EDIT PHASE AND
04707120101FORCE AN OBJECT FILE TO BE PRODUCED [-c] (y/n):
05709100101COMPILE FOR "sdb" DEBUGGER [-g] (y/n):
06711100101INVOKE OBJECT-CODE OPTIMIZER [-O]  (y/n):
07713100030SEARCH  LIBRARIES, libx.a. [-l]  (ENTER x):
08715100030ADDITIONS TO THE COMMAND LINE:

00080
02070-o a.out
04010c
05010g
06010O
07010
08092


This is the "typical" form of the "cc" command and it only contains
a few of the most frequently used options.  "cc" is the interface
for the C Compilation System.  The compilation tools consist of a
preprocessor, compiler, optimizer, assembler, and link editor.
The "cc" command processes the supplied options and then executes
the various tools with the proper arguments.  In typical usage, 
"cc" compiles source files whose names end with ".c" and produces
an executable object file, "a.out".
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
04This field when invoked, will write the object file(s)
into your directory.  The object files will have ".o"
as a suffix and have the same prefix as your ".c" 
files.   Normally, "cc" links object files, and, in
case only one file is compiled, deletes the ".o" file.  [-c]
05Causes the compiler to generate additional information needed 
for the use of "sdb(1)", a symbolic debugging program.  [-g]
06Do compilation phase optimization.  This option will
not have any affect on .s files.  [-O]
07Search a library, "libx.a", where "x" is up to nine characters.
A library is searched when its name is encountered, so the
placement of the arguments(s), "x", is significant.  By default,
libraries are located in LIBDIR (usually /lib) or LLIBDIR
(usually /usr/lib.)  

ASSIST does no validation on this field.   [-l]
08ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0204
0402
0506
0605


040760n
050949n
061152n

0002
003420001010
0203
0205
0232if test -d $F02;  then echo "$F02 IS A DIRECTORY";fi
0405
0404yYnN
0504yYnN
0505
0604yYnN
0605


001110
011110
021010-o 
041011y-cY-cnN
051011y-gY-gnN
061011y-OY-OnN
071110-l
081110




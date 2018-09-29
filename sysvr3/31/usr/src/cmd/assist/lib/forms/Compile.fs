#ident	"@(#)forms.files:Compile.fs	1.4"
400COMPILERS
00001150000COMPILERS, INTERPRETERS, AND ASSEMBLERS
01703150101.... Pattern scanning and processing language
02704140101..... C compiler
03705140101..... Link editor for common object files
04706140101..... Command programming language
05008130000MORE COMPILERS, INTERPRETERS, AND ASSEMBLERS
06710140101..... Common assembler

The Compilers, Interpreters, and Assemblers menu
lists the commands you use to compile or interpret
code and link object files for execution.

01"awk" is a programmable UNIX system filter that
does pattern scanning and language processing.  It
includes text substitution, arithmetic, variables,
built-in functions, and a programming language that
looks similar to C. 
02"cc" is the interface for the C Compilation System.
The compilation tools consist of a preprocessor,
compiler, optimizer, assembler and link editor.  The
"cc" command processes the supplied options and then
executes the various tools with the proper arguments.
In the typical usage, "cc" compiles source files
whose names end on ".c" and produces an executable
object file, "a.out".
03"ld" is a link editor that combines several object
files into one, performs relocation, resolves
external symbols, and supports symbol table infor-
mation for symbolic debugging.  In the simplest case,
the names of several object programs are given, and
"ld" combines them, producing an object module that
can either be executed or used as input for a sub-
sequent "ld" run.  The output of "ld" is left in
"a.out".  By default this file is executable if no
errors occurred during the load.  If any input file
is not an object file, "ld" assumes it is either
an archive or library or text file containing link
editor directives.  Normally, "ld" is not invoked
directly by the user, but by the compiler, "cc".
04"sh" is the UNIX System command interpreter.  "sh"
is used to execute one or more commands from a file,
the command line, or your terminal.
06"as" assembles a file.  "as" names one output object
file by removing the .s suffix (if present) from the
input file name and appending a .o suffix.  Whenever
possible, you should access the assembler through a
compilation interface program (such as "cc") rather
than using "as" directly.  

010311awk
020411cc
030511ld
040611sh
061011as

01awk
02cc
03ld
04sh
06


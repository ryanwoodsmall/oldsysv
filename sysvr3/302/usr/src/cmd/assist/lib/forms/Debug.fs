#ident	"@(#)forms.files:Debug.fs	1.4"
400DEBUG
00001250000DEBUG
01703130101............. Symbolic debugger for C and Fortran
02704240101.. Brief introduction to the "sdb" symbolic debugger
03006190000MORE COMMANDS FOR DEBUGGING
04708120101.............. C program beautifier that spaces and indents code
05709150101........... Generate C flow graph
06710160101.......... Debugs C programs
07711150101........... Generate C program cross-reference
08712140101............ Checks C programs
09713150101........... Print shell input lines as they are read
10714150101........... Print shell commands as they are executed

The Debug menu lists commands that help you find
errors in code.

01"sdb" is a debugger for C and Fortan77 programs.
You can use "sdb" to examine "core" images of
programs that aborted at runtime and to monitor
and control program execution.
02ASSIST's "sdb" walkthru is designed to get you started
with a basic set of "sdb" concepts and commands.


"sdb" is a debugger for C and Fortran77 programs.
You can use "sdb" to examine "core" images of
programs that aborted at runtime and to monitor
and control program execution.
04"cb" (C program beautifier) reformats source code
files, with spacing and indentation that reflects
the structure of the code.  This program is useful
not only to make source code look better, but also
to help detect errors.
05"cflow" is used to analyze external references in
programs.  You may include any combination of C,
yacc, lex, assembler, and object files, which will
be processed as needed.  The output of "cflow" is
a graphical representation of external references
displayed on your screen.
06"ctrace" is a debugger for C programs that allows you
to follow the execution of a C program, statement by
statement.  The effect is similar to executing a shell
procedure using "sh -x".  "ctrace" reads the C program,
inserts statements to print the text of each executable
statement and print the values of all variables refer-
enced or modified, and writes the modified program to
the standard output.  You put the output of "ctrace"
in a temporary file and compile and execute this file.
07"cxref" creates a table of cross-references.
"cxref" takes as input a number of C source
files, and produces as output a list of all
automatic, static, and global symbols, along
with the declaring reference of each.
08"lint" attempts to detect features of C program
files that are likely to be bugs, non-portable,
or wasteful.  Among the things currently checked
are unreachable statements, loops not entered at
the top, automatic variables declared and not used,
and various aspects of functions usage.
09"sh" with the "-v" (for verbose) option prints
each shell input line as it reads and executes it.
10"sh" with the "-x" (for execute) option prints each
command with its arguments as the command is executed
by "sh".  Thus, the "-x" option helps you to debug
shell scripts by showing you what the commands and
argument values are as the script is being interpreted
by "sh".

010309sdb
020409assist sdbwalk
040809cb
050909cflow
061009ctrace
071109cxref
081209lint
091309sh -v
101409sh -x

01sdb
02sdbwalk
04
05
06
07
08
09
10


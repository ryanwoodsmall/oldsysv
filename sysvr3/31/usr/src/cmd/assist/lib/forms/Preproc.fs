#ident	"@(#)forms.files:Preproc.fs	1.4"
400PRE-PROCESSORS 
00001170000PRE-PROCESSORS AND CODE GENERATION
01703210101...... Generate code for a lexical analysis of text
02704220101..... Yet another compiler compiler
03009100000MORE COMMANDS FOR PRE-PROCESSING AND CODE GENERATION
04711210101...... C language preprocessor
05712200101....... Macro processor
06713240101... Regular expression compiler

The Pre-processors and Code Generation menu lists
commands for pre-processing C code and generating
code from macros.

01"lex" generates programs for simple lexical analysis 
of text.  "lex" produces a C code subroutine as output
that you compile and combine with other programs.  You 
can use the processing done by "lex" as the first step 
in creating a compiler-type program.  In addition, you
can use output from "lex" as a preprocessing tool for
many different software generation functions.
02"yacc" is a utility for creating parser subroutines.  
It is primarily used in the generation of software
compilers.  "yacc" produces C source code for a parser 
subroutine; the parser subroutine is then compiled and 
finally used with a program that calls the subroutine
to parse input.
04"cpp" invokes the C preprocessor.  Normally, this
preprocessor is invoked automatically, when you
execute the "cc" command.  Due to possible future
changes in the way the program compilation commands
work, we suggest that you use "cpp" through the "cc"
command.
05The "m4" macro processor is a front end for the C
and rational Fortran (Ratfor) programming languages.
The "#define" statement in the C language and the
analogous "define" in Ratfor are examples.  Besides 
the basic replacement of one string of text for
another, the "m4" macro processor provides the
following features: arguments, arithmetic capabili-
ties, file manipulation, conditional macro expansion,
and string and substring functions.  
06"regcmp" is a special-purpose C code generator.  It
generates a code fragment that initializes an octal
array.  This array is used by "regex(3X)" as the
compiled pattern against which a subject string is
compared.  Using "regex" in this way is faster and
results in smaller programs than when "regcmp(3X)"
is used inside the program to generate the octal
array at runtime.

010317lex
020417yacc
041117cpp
051217m4
061317regcmp

01lex
02yacc
04
05
06


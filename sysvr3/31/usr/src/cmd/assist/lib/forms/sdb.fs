#ident	"@(#)forms.files:sdb.fs	1.4"
00000sdb
00701060101OBJECT FILE:
01703060001CORE FILE:
02005060000DIRECTORY(IES) CONTAINING SOURCE
03706080001FILES USED TO BUILD THE OBJECT FILE:
04708060001SUPRESS WARNING MESSAGES ABOUT THE OBJECT FILE [-W] (y/n) :
05710060101PERMIT OVERWRITING OF LOCATIONS IN THE OBJECT FILE [-w] (y/n) :
06712060030ADDITIONS TO THE COMMAND LINE:

00040
01050
03060
04030-W
05020-w
06072

"sdb" is an interactive command
Use q<CR> to exit "sdb"

"sdb" is a debugger for C and Fortan77 programs.  You can use "sdb" to 
examine "core" images of programs that aborted at runtime and to 
monitor and control program execution. 
"sdb" is an interactive command with its own command interface.  Use
the ASSIST walkthru for "sdb" to learn some basic "sdb" commands and
concepts.

00The object file is a file containing an executable program.  You should
use the -g (debug) option when compiling the source code.  The default
object file is "a.out".  The ASSIST command form shows the "a.out" file
when there is an executable file named "a.out" in the current directory.
The command form also shows "a.out" on the command line.  
01The core file is a core image file produced if the object file
aborts when it is executed.  The default for the core file is "core."
Enter a - sign here to cause "sdb" to ignore core images.
03The default location for the source files that produced the
object file is the current directory.  If the source code is
in a different directory, enter the name of that directory
here.  If the source is in more than one directory, use a
colon (:) separated list of directories. 
04By default, "sdb" provides warnings if the source files are newer than
the object file, or if "sdb" cannot find the source files.  Answer "y"
to this option to turn off this checking. [-W]
05This option permits "sdb" to overwrite locations in the object file. [-w]
06ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.


0100
030100

000119if test -r a.out -a -x a.out; then echo "a.out";fi
010317if test -r core; then echo "core";fi
030645current_directory
040866n
051070n

003420001000
0032if test -z "`file $F00 | grep execu`"; then echo "$F00 is not an executable file";fi
0019
0115-
013410101010
0116
0119
0316
0332for i in `echo $F03 | sed -e 's/:/ /g' `; do if test ! -d $i; then echo "$i NOT A DIRECTORY"; exit; elif test ! -x $i; then echo "CANNOT ACCESS $i DIRECTORY"; exit; fi; done
0417yYnN
0404yYnN
0504yYnN


001010if test -r a.out -a -x a.out; then echo "a.out";fiif test -r a.out -a -x a.out; then echo "a.out";fi
011010if test -r core; then echo "core";else echo ""; fiif test -r core; then echo "core";fi
031010current_directory
041011y-WY-WnN
051011y-wY-wnN
061110




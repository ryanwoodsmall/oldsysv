#ident	"@(#)forms.files:unixwalk.fs	1.2"
400UNIX SYSTEM WALKTHRU
00001240000UNIX SYSTEM WALKTHRU
01100000101
02100000101
03100000101
04100000101
05100000101
06100000101
07100000101

You can use this menu to select the portion(s) of the UNIX
System Fundamentals walkthru in which you are interested.

01Select this item if you want to receive the complete walkthru.
This includes Files and Directories, Standard Input and Output,
Output Redirection, and Input Redirection
02Select this item if you only want to receive the portion
of the walkthru that deals with UNIX system file and directory
concepts.  You will learn about file and directory naming,
directory structure, and some useful commands that act on files
and directories.
03This section of the walthru deals with the concepts of
standard input and output.  An understanding of these concepts
is critical to understanding redirection of input and output.
04Select this section to learn how to save the output of commands.
You will learn how to place the output of a command into a file,
and also how to send the output of one command to another command
for further processing.
05This section of the walthru covers redirection of output
from a command to a file.
06This section of the walthru describes how to send the
output of one command to another command for further processing.
07This section describes how to allow certain commands to take
input from a file.

010317Complete Walkthru
020519Files and Directories
030719Standard Input and Output
040919Output Redirection
051121Redirection to a File (">")
061321Piping to a Program ("|")
071519Input Redirection

01($ASSISTBIN/mscript -m -s un.complete; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the walkthru menu \\c"; read return; fi; exit $i)
02($ASSISTBIN/mscript -m -s un.filedir; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the walkthru menu \\c"; read return; fi; exit $i)
03($ASSISTBIN/mscript -m -s un.stdio; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the walkthru menu \\c"; read return; fi; exit $i)
04($ASSISTBIN/mscript -m -s un.redirout; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the walkthru menu \\c"; read return; fi; exit $i)
05($ASSISTBIN/mscript -m -s un.redirfile; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the walkthru menu \\c"; read return; fi; exit $i)
06($ASSISTBIN/mscript -m -s un.redirpipe; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the walkthru menu \\c"; read return; fi; exit $i)
07($ASSISTBIN/mscript -m -s un.redirin; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the walkthru menu \\c"; read return; fi; exit $i)


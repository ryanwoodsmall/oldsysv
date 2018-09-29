#ident	"@(#)forms.files:sdbwalk.fs	1.3"
400sdbwalk
00003280000SDB WALKTHRU MENU
01706200101
02707230101
03708230101
04709230101
05710230101
06711230101

"sdb" is a debugger for C and Fortran77 programs.  You can use "sdb" to 
examine core images of programs that aborted at runtime and to monitor 
and control program execution.  
  
ASSIST's "sdb" walkthru is designed to get you started with a basic
set of "sdb" concepts and  commands.

01The complete "sdb" walkthru will introduce you to the following "sdb"
concepts and commands:  display your source files in "sdb"; change the
current line; set, delete, and use breakpoints; execute commands;
examine variable values; use several source files; and debug core dumps. 
02This part of the "sdb" walkthru describes how to 
display portions of your source file and how to 
change the current line of your source file.  
03This part of the "sdb" walkthru introduces 
breakpoint debugging and describes setting and 
deleting break points.  
04This part of the "sdb" walkthru shows you how to run your object code
from within "sdb" and how to examine variable values. 
05This part of the "sdb" walkthru shows you how to switch between
source code files when your object code is based on more than one
source code file. 
06This part of the "sdb" walkthru explains what a "core dump" is
and shows how to use the "sdb" trace command.

010620Complete "sdb" Walkthru
020723Moving Through a Source File
030823Breakpoint Debugging
040923Executing Commands and Getting Variable Values
051023Debugging Code with More Than One Source File
061123Debugging Core Dumps

01($ASSISTBIN/mscript -m -s sdb.complete; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the sdbwalk menu \\c"; read return; fi; exit $i)
02($ASSISTBIN/mscript -m -s sdb.search; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the sdbwalk menu \\c"; read return; fi; exit $i)
03($ASSISTBIN/mscript -m -s sdb.break; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the sdbwalk menu \\c"; read return; fi; exit $i)
04($ASSISTBIN/mscript -m -s sdb.execute; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the sdbwalk menu \\c"; read return; fi; exit $i)
05($ASSISTBIN/mscript -m -s sdb.multi; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the sdbwalk menu \\c"; read return; fi; exit $i)
06($ASSISTBIN/mscript -m -s sdb.core; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the sdbwalk menu \\c"; read return; fi; exit $i)


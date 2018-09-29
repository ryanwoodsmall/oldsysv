#ident	"@(#)forms.files:assistwalk.fs	1.2"
400ASSIST WALKTHRU
00003150000INTRODUCTION TO ASSIST
01704180101
02705200101
03706200101
04707200101
05708200101

ASSIST is a menu interface designed to help UNIX system users locate 
and use UNIX system commands.  The complete walkthru for ASSIST 
includes introductory information on what ASSIST does and on the basic 
ASSIST commands.  It includes the walkthrus for using ASSIST's menus, 
command forms, pop-up help, and pop-up menu.  

01The complete walkthru for ASSIST includes introductory information on 
what ASSIST does and on the basic ASSIST commands.  It includes the 
walkthrus for using ASSIST's menus, command forms, pop-up help, and 
pop-up menu.  
02ASSIST's menus provide a functional categorization of UNIX system 
commands.  This walkthru shows several ASSIST menus and teaches the 
basic commands used to move thru ASSIST's menus.  
03ASSIST's command forms are interactive, full-screen versions of UNIX 
system commands.  This walkthru shows an example of an ASSIST command 
form and teaches the basic commands needed to use an ASSIST command 
form.  
04ASSIST provides three types of pop-up help.  This walkthru will show 
you examples of each type of help and will teach you the commands you 
use to ask for the different types of help.  
05ASSIST's pop-up menu gives you access to more ASSIST functions.  This 
walkthru will tell you about these functions and show you how to use 
some of them.  

010418Complete Introductory Walkthru
020520Menus
030620Command Forms
040720Pop-up Help
050820Pop-up Menu Functions

01($ASSISTBIN/mscript -m -s as.intro; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the assistwalk menu \\c"; read return; fi; exit $i)
02($ASSISTBIN/mscript -m -s as.menu; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the assistwalk menu \\c"; read return; fi; exit $i)
03($ASSISTBIN/mscript -m -s as.cf; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the assistwalk menu \\c"; read return; fi; exit $i)
04($ASSISTBIN/mscript -m -s as.help; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the assistwalk menu \\c"; read return; fi; exit $i)
05($ASSISTBIN/mscript -m -s as.popmenu; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the assistwalk menu \\c"; read return; fi; exit $i)


#ident	"@(#)forms.files:viwalk.fs	1.3"
400viwalk
00002240000VI WALKTHRU
01704200101
02705230101
03706230101
04707230101
05708230101
06709230101
07710230101
08712200030
09714200101

"vi" is a full-screen text editor.  When you use "vi", the changes you
make are reflected on the screen.  You can use "vi" to create and edit 
any type of text file, including mail messages, documents that will be 
formatted by a command such as "mm", programs in languages such as C, 
and shell scripts.  
    
ASSIST's "vi" walkthru is designed to get you started with a basic set
of "vi" editing commands and concepts.  

01The complete walkthru includes some introductory information; an 
optional demonstration of "vi"; and walkthrus covering basic "vi" 
commands and concepts needed to position the cursor, delete text, undo 
editing changes, add text, save editing changes, and quit "vi".  
02The "vi" demonstration shows you how "vi" behaves.  Many 
people find a demonstration of "vi" helpful, especially if 
they have never used "vi". 
03This part of the "vi" walkthru shows how to position the cursor
by characters, lines, and words.  It also shows how to multiply
"vi" commands. 
04This part of the "vi" walkthru shows how to delete characters,
words, and lines. 
05This part of the "vi" walkthru shows how to undo the last
text modification 
06This part of the "vi" walkthu shows how to add text to the
right and to the left of the current cursor position; how to start
a new file; and how to open a line above and below the current line.
07This part of the "vi" walkthru shows how to save editing changes
and how to quit "vi". 
08This walkthru will tell you about two of "vi"'s frequently used
options: line numbering and show mode.  
09This item lists each command shown in the "vi" walkthru
together with a brief description of the command. 

010420Complete Introductory Walkthru
020523Demonstration of "vi"
030623Cursor Positioning
040723Deleting Text
050823Undoing Changes
060923Adding Text
071023Saving Changes and Quitting
081220Setting "vi" Options
091420Summary of Basic "vi" Commands

01($ASSISTBIN/mscript -m -s vi.complete; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)
02($ASSISTBIN/mscript -m -s vi.demo; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)
03($ASSISTBIN/mscript -m -s vi.cursor; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)
04($ASSISTBIN/mscript -m -s vi.delete; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)
05($ASSISTBIN/mscript -m -s vi.undo; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)
06($ASSISTBIN/mscript -m -s vi.add; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)
07($ASSISTBIN/mscript -m -s vi.manip; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)
08($ASSISTBIN/mscript -m -s vi.options; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)
09($ASSISTBIN/mscript -m  $ASSISTLIB/scripts/vicommands; i=$?;if test $i -eq 3; then echo "Strike the RETURN key to return to the viwalk menu \\c"; read return; fi; exit $i)


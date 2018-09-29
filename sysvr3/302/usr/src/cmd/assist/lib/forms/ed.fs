00000ed
00703060001FILE NAME:
01005060030SUPPRESS THE PRINTING OF CHARACTER COUNTS, DIAGNOSTICS,
02706080101AND THE ! PROMPT [-s] (y/n):
03708060001STRING TO BE USED AS PROMPT [-p]:
04710060001ENCRYPT FILE AS IT IS BEING WRITTEN [-x] (y/n):
05712060030ADDITIONS TO THE COMMAND LINE:

00050
02020
03030
04040
05062

"ed" is an interactive command.

Type "Q" on a line by itself 
to exit the editor without saving your editing.

Type "w" on a line by itself, 
then type "q" on a line by itself
to exit the editor and save your editing.

"ed" is a line-oriented text editor that allows you
to create and edit files.  Editing with "ed" is
line-by-line rather than full-screen (as in "vi").
You can use "ed" on both hard-copy and video display
terminals.

00Enter the name of the file you want to edit here.
02If you specify a "y" for this field, it will suppress typical
messages from the editor.  The messages that will be suppressed
are the character counts printed by the editor "e," "r," and "w"
commands, diagnostic messages from the editor "e" and "q" commands,
and the ! prompt after a !shellcommand was executed.  [-s]

(If the version of the UNIX operating system that is running on
your machine is any release earlier than UNIX System V Release 3.0, 
this option will appear on the command line as "-" not "-s".)
03If you want a prompt string to identify each time the 
editor is waiting for an input command from you, 
specify a prompt string here.  If you do not specify
anything here, by default the editor will not print
prompts. [-p]
04If you type a "y" for this field, the file you are 
creating in the editor will be encrypted as you are
writing it.  The file will require an encryption key
to be read.  See the "crypt" command.
05ASSIST appends anything you type here to the command line.  
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field,
you should use it with caution.



020637n
041054n

000114
0204yYnN
0235
030116
0404yYnN


001010
011110
021011yif test `/bin/uname -r | /usr/bin/cut -c1` -lt 3; then /bin/echo -; else /bin/echo -s; fiYif test `/bin/uname -r | /usr/bin/cut -c1` -lt 3; then /bin/echo -; else /bin/echo -s; finN
031010-p 
041011y-xY-xnN
051110




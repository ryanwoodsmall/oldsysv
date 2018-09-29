00000ed
00703060001FILE NAME:
01005060030SUPPRESS THE PRINTING OF CHARACTER COUNTS, DIAGNOSTICS,
02706080101AND THE ! PROMPT [-s] (y/n):
03708060001STRING TO BE USED AS PROMPT [-p]:
04910020030ENCRYPTION/DECRYPTION OF FILES
05011060030READ IN FILE ASSUMING FILE IS ENCRYPTED AND
06712080101WRITE OUT FILE IN ENCRYPTED FORM [-C] (y/n):
07013060030READ IN FILE HAVING "ed" MAKE EDUCATED GUESS TO SEE IF
08714080101FILE IS ENCRYPTED AND WRITE OUT FILE IN ENCRYPTED FORM [-x] (y/n):
09717060030ADDITIONS TO THE COMMAND LINE:

00050
02020
03030
06040
08040
09062
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
and the ! prompt after a !shellcommand was executed.  [-s]
03If you want a prompt string to identify each time the 
editor is waiting for an input command from you, 
specify a prompt string here.  If you do not specify
anything here, by default the editor will not print
prompts. [-p]
06You use a "ed" encryption option if you want to encode the
file you are creating so that other people cannot read the
file unless they know your encryption key.  You also use an
encryption option to read files that you have previously
encrypted.  When you execute the "ed" command, "ed" will
prompt you for an encryption keyword.  Use ASSIST's command
form for the "crypt" command to learn more about encrypting
files. 

The "-C" encryption option assumes that the files named are
encrypted.  [-C]
08You use a "ed" encryption option if you want to encode the
file you are creating so that other people cannot read the
file unless they know your encryption key.  You also use an
encryption option to read files that you have previously
encrypted.  When you execute the "ed" command, "ed" will
prompt you for an encryption keyword.  Use ASSIST's command
form for the "crypt" command to learn more about encrypting
files. 

The "-x" encryption option does not assume that the file
named is encrypted.  Instead, it examines the contents of
the file and tries to make an intelligent decision as to
whether or not the file is encrypted.  [-x]
09ASSIST appends anything you type here to the command line.  
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field,
you should use it with caution.

0608
0806


020637n
061253n
081475n

000114
0204yYnN
030116
0604yYnN
0605
0805
0804yYnN


001010
011110
021011y-sY-snN
031010-p 
041110
051110
061011y-CY-CnN
071110
081011y-xY-xnN
091110




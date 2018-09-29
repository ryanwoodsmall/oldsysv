00000wc
00701060130NAME OF INPUT FILE(S):
01703060001REDIRECT OUTPUT TO:
02705060001PRINT THE NUMBER OF CHARACTERS [-c] (y/n):
03707060001PRINT THE NUMBER OF LINES [-l] (y/n):
04709060001PRINT THE NUMBER OF WORDS [-w] (y/n):
05711060030ADDITIONS TO THE COMMAND LINE:

00030
01060> 
02021c
03021l
04021w
05112


"wc" counts the number of lines, words,
and/or characters in a file. Without any
options, "wc" lists all three counts.

00"wc" requires at least one filename as
input.   "wc" prints the filename with
each count of lines, words and/or characters.
01If you type a filename here, the output of this command will go to the 
file.  ASSIST will warn you if the file you type already exists.  If it 
exists, redirecting output to it will destroy the previous contents of 
the file.  (See the UNIX system walkthru for a complete discussion of 
redirection.) 
02With this option, "wc"  only prints a count of the
the characters in the input file or files. [-a]
03With this option, "wc" prints a
count of lines in the input file or files. [-l]
04If you use this option,  "wc" prints a
count of the words in the input file or files. [-w]
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field,
you should use it with caution.



020549n
030744n
040944n

0002
003410001010
0103
0113
0107
0132if test -d $F01;then echo "$F01 IS DIRECTORY";fi
0204yYnN
0304yYnN
0404yYnN


001110
011010> 
020001y-cY-cnN
030001y-lY-lnN
040001y-wY-wnN
051110




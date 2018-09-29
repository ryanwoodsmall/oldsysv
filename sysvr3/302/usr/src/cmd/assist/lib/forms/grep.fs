#ident	"@(#)forms.files:grep.fs	1.3"
00000grep
00701050030NAME(S) OF FILE(S) TO BE SEARCHED:
01703050101EXPRESSION SEARCHED FOR:
02705050101ONLY PRINT LINES THAT DO NOT MATCH [-v] (y/n):
03707050101ONLY PRINT COUNT OF MATCHING LINES [-c] (y/n):
04709050101IGNORE UPPER/LOWER CASE DISTINCTION DURING COMPARISON [-i]  (y/n):
05711050101ONLY PRINT NAMES OF FILES CONTAINING EXPRESSION [-l] (y/n):
06713050101PRECEDE LINE BY LINE NUMBER IN FILE [-n] (y/n):
07715050101PRECEDE LINE BY BLOCK NUMBER [-b] (y/n):
08717050101SUPPRESS ERROR MESSAGES FOR UNREADABLE FILES [-s] (y/n):
09719050030ADDITIONS TO THE COMMAND LINE:

00020
01010
02001v
03001c
04001i
05001l
06001n
07001b
08001s
09032

By not entering a file name, you have requested that the "grep" command
take input from your terminal.  To end input, type ^D.00

"grep" searches the input files for lines matching 
a pattern.  The pattern can be either an ordinary 
string, or a "limited regular expression" (explained 
in the help for the "EXPRESSION SEARCHED FOR" item).

00Enter the name(s) of file(s) you want to search through.
If you enter multiple files, the command will process each in 
the order specified.  If you leave this field blank, the command 
will take input from standard input.
01You can search either for ordinary strings or for "limited regular 
expressions" -- strings containing the characters ^, ., $, [, ], *, or \.
It is strongly encouraged to enclose your string in single quotes,
because otherwise the shell interpretation of the string will
be searched for rather than the string itself.

                Meaning of Special Characters 

        ^              beginning of a line
        $              end of a line
        .              any single character
        c*             any number of c's (c is a character)
        [abc..]        a OR b OR c OR ... 

Use \ to escape the special meaning of these characters.
02All lines but those matching are printed. This option 
reverses the way "grep" normally functions [-v].
03Useful to find out number of occurrences 
of a pattern in a file [-c].
04E.g., Ab matches ab, AB, aB, and Ab [-i].
05Useful to find out quickly what file to look into more closely [-l].
06Each line is preceded by its relative number in
the file. Useful for later editing [-n].
07Useful for locating disk block numbers by context.
08Useful when you use "grep" inside a shell script [-s].
09ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not do any error checking on this field,
you should use it with caution.

0203
0302


020552n
030752n
040972n
051165n
061353n
071546n
081762n

003420001010
0126
0127;&()|^<> 	'"
0204yYnN
0205
0304yYnN
0305
0404yYnN
0504yYnN
0604yYnN
0704yYnN
0804yYnN


001110
011010
020001y-vY-vnN
030001y-cY-cnN
040001y-iY-inN
050001y-lY-lnN
060001y-nY-nnN
070001y-bY-bnN
080001y-sY-snN
091110




#ident	"@(#)forms.files:fgrep.fs	1.2"
00000fgrep
00701050030NAME(S) OF FILE(S) TO BE SEARCHED:
01903020030SPECIFICATION OF SEARCH STRINGS
02704050001STRING SEARCHED FOR:
03705050001SEARCH FOR A SPECIAL STRING [-e]:
04706050001TAKE THE LIST OF STRINGS FROM FOLLOWING FILE [-f]:
05708050101ONLY PRINT LINES THAT DO NOT MATCH [-v] (y/n):
06709050101ONLY PRINT COUNT OF MATCHING LINES [-c] (y/n):
07710050101IGNORE UPPER/LOWER CASE DISTINCTION DURING COMPARISON [-i] (y/n):
08711050101ONLY PRINT NAMES OF FILES CONTAINING EXPRESSION [-l] (y/n):
09712050101PRECEDE LINE BY LINE NUMBER IN FILE [-n] (y/n):
10713050101PRECEDE LINE BY BLOCK NUMBER [-b] (y/n):
11714050001PRINT ONLY LINES THAT EXACTLY MATCH SEARCH STRING [-x] (y/n):
12716050030ADDITIONS TO THE COMMAND LINE:

00120
02090
03090
04090
05001v
06001c
07001i
08001l
09001n
10001b
11001
12132

By not entering a file name, you have requested that the "fgrep" command
take input from your terminal.  To end input, type ^D.00

"fgrep" searches the input files for lines matching a string.
The pattern must be an ordinary string, containing no
"special characters" as "grep" and "egrep" do.

00Enter the name(s) of file(s) you want to search through.
If you enter multiple files, the command will process each in 
the order specified.  If you leave this field blank, the command 
will take input from standard input.
02Type any string here.  If the string contains characters
that are special to the shell, enclose the string in quotes;
alternatively, you can enter the string in the next field.
03Use this option for strings that either contain characters
special to the shell or for strings that start with a "-" sign.
If the string contains spaces, enclose it by single quotes [-e].
04You can store any number of search strings in a file, separated
by newlines [-f].
05All lines but those matching are printed. This option 
reverses the way "fgrep" normally functions [-v].
06Useful to find out the number of occurrences of a pattern in a file [-c].
07E.g., Ab matches ab, AB, aB, and Ab [-i].
08Useful to find out quickly what file to look into more closely [-l].
09Each line is preceded by its relative number in
the file. Useful for later editing [-n].
10Useful for locating disk block numbers by context [-b].
11Make sure that you enclose your search string by quotes [-x].
12ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not do any error checking on this field,
you should use it with caution.

020304
030204
040203
06091008
08060910
090608
100608


050852n
060952n
071071n
081165n
091253n
101346n
111467n

003420001010
0227;&()|^<> 	'"
0205
0231
0218-
0331
0305
0431
0405
043420001010
0504yYnN
0604yYnN
0605
0704yYnN
0804yYnN
0805
0904yYnN
0905
1004yYnN
1005
1104yYnN


001110
011110
021010
031010-e 
041010-f 
050001y-vY-vnN
060001y-cY-cnN
070001y-iY-inN
080001y-lY-lnN
090001y-nY-nnN
100001y-bY-bnN
110001y-xY-xnN
121110

020304
030204
040203



#ident	"@(#)forms.files:diff.fs	1.5"
00000diff
00701060101NAME OF FIRST FILE:
01703060101NAME OF SECOND FILE:
02705060001REDIRECT OUTPUT TO:
03707060001IGNORE TRAILING BLANKS [-b] (y/n):
04709060001PRODUCE SCRIPT CONVERTING FIRST FILE TO SECOND [-e] (y/n):
05711060001PRODUCE SCRIPT CONVERTING SECOND FILE TO FIRST [-f] (y/n):
06713060001FAST, PARTIAL COMPARISON ONLY [-h] (y/n):
07715060030ADDITIONS TO THE COMMAND LINE:

00050
01060
02070> 
03031b
04011e
05021f
06041h
07072

You are about to execute an interactive command.
The command will expect you to enter input from your
terminal; when you have finished entering your input,
type ^D.00-

"diff" tells you what lines you must change to
make two files identical.  "diff" can also create
scripts that you can use with "ed" to change the files.

00Enter the first file to be compared.  If you enter a 
directory name here, the command will look for a file 
in that directory that has the same name as the file 
entered in the "NAME OF SECOND FILE" field.
01Enter the second file name here.
02By default, the "diff" command places its output on your terminal.
If you want to save the output in a file, enter a file name here.
If the file exists, the current contents will be destroyed.
03Selection of this option has two effects.  First, "diff" will
ignore any blanks at the end of a line in comparisons.  Second, 
"diff" will treat multiple consecutive blanks identically 
to a single blank.  [-b]
04If you select this option "diff" will produce a script of "ed" commands,
that may be used to change the first file into the second.  If
you are going to use this option, you may want to redirect output
to a file, using the "REDIRECT OUTPUT TO" field.
Note that to really use this output with "ed," you will have to add
a "w" and a "q" (each on separate lines) at the end of the file. [-e]
05If you select this option "diff" will produce a script of 
commands converting the second file into the first.  
However, the script cannot be used with "ed" (unlike the field 
above).  If you want an "ed"-compatible script converting the 
second file to the first, switch the names of "NAME OF FIRST FILE" 
and "NAME OF SECOND FILE," and select the option immediately 
above this one.  [-f]
06If you select this option, you will be able to compare files of
unlimited length.  However, only short, well separated differences
between the two files will be detected. [-h]
07ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

040506
050406
060405


030741n
040965n
051165n
061348n

0001
0015-
0032if [ "$F00" = "-" -a "$F01" = "-" ]; then echo " YOU CANNOT HAVE HYPHENS IN BOTH FIELDS ";fi
0101
0113
013420101010
0107
0132if [ "$F00" = "-" -a "$F01" = "-" ]; then echo " YOU CANNOT HAVE HYPHENS IN BOTH FIELDS ";fi
0115-
0201
0213
0203
0304yYnN
0405
0404yYnN
0505
0504yYnN
0605
0604yYnN


001110
011110
021010> 
030001y-bY-bnN
040001y-eY-enN
050001y-fY-fnN
060001y-hY-hnN
071110




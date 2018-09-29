#ident	"@(#)forms.files:awk.fs	1.4"
00000awk
00701060030INPUT FILE(S):
01703060001NAME OF FILE CONTAINING "awk" PROGRAM [-f] :
02705060001COMMAND-LINE PROGRAM:
03707060001REDIRECT OUTPUT TO:
04709060101FIELD SEPARATOR [-F] :
05711060030COMMAND-LINE PARAMETER ASSIGNMENT(S):
06713060030ADDITIONS TO THE COMMAND LINE:

00050
01020
02032
03060
04012
05040
06072

"awk" will take input from the standard input 
when you execute the current command line.

Use ^D to end standard input.  00-

"awk" is a programmable UNIX system filter that
does pattern scanning and language processing.  It
includes text substitution, arithmetic, variables,
built-in functions, and a programming language that
looks similar to C. 

00"awk" will scan each file you specify here for lines that match
any of the patterns specified in the "awk" program. 
If you have no input files named here, then "awk" will read input
from the standard input.  Also, "awk" will take input from
the standard input if you use a "-" in this field.
01To use "awk" you must specify an "awk" program either in a file
or on the command line for "awk".  If your "awk" program is in a file,
put the name of the file here.  ASSIST will check to see if the
file you name is readable, but ASSIST will not check whether the 
"awk" program in your file is correct. [-f]

The following is an example of a short "awk" program:
BEGIN {sum = 0}
{sum = sum + $2}
END {
    print "The sum of the second column is ", sum
    print "and the average of the second column is ", sum/NR }
02To use "awk" you must specify an "awk" program either in a file or on
the command line for "awk".  Use this field if you want to put your "awk"
program on the command line.  ASSIST does not check whether the "awk"
program you enter here is a syntactically correct "awk" program.

USE single quotes around your command line program.

For example, you could print all the lines in a file that were longer
than 72 characters by putting 'length > 72' in this field.
03If you type a file name here, the output of this command will go to the
file.  ASSIST will warn you if the file already exists.  If the named
file exists, re-directing output to it will destroy the previous
contents of the file.  (See the UNIX system walkthru for a more complete
discussion of redirection.)
04"awk" treats each line of your input file(s) as made up of fields
that are separated by white space.  You can change the default
field separator to any single character, such as a ":"  [-F]

The ASSIST command form for "awk" will place ' signs on each side of any
character you enter in this field; therefore do not type ' before or
after your character. 
05"awk" allows you to put parameter definitions on the command line.  
These parameters (e.g. sum=0 ) are passed to "awk" from the command line.
06ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0102
0201


040929white_space

0015-
0032if test -d $F00; then /bin/echo "$F00 IS A DIRECTORY"; fi
0105
013420001010
0131
0205
0231
0217''
0303
0332if test -d $F03; then /bin/echo "$F03 IS A DIRECTORY"; fi
04011
0532/bin/echo $F05 | /bin/grep '[a-zA-Z0-9_]=' > /dev/null; if test $? -ne 0; then /bin/echo "PARAMETER EXPRESSION $F05 NOT IN FORM x=...";fi


001110
011010-f 
021110
031010> 
041010-F''white_space
051110
061110

0102
0201



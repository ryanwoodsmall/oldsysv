00000sort
00701060130NAME OF FILE(S):
01702060001NAME OF THE OUTPUT FILE [-o]:
02904020030ORDERING RULES
03705060001"DICTIONARY" ORDER [-d] (y/n):
04706060001"ARITHMETIC VALUE" ORDER [-n] (y/n):
05707060001ORDER AS MONTHS OF THE YEAR [-M] (y/n):
06908060030ASCII ORDER IGNORING CHARACTERS
07709080001OUTSIDE THE RANGE OF 040 - 0176 [-i] (y/n):
08710060001REVERSE ORDER [-r] (y/n):
09711060001TREAT LOWER CASE AS UPPER CASE FOR COMPARISON [-f] (y/n):
10912020030SORTING ON A FIELD
11913060030NUMBER OF FIELDS TO SKIP AND NUMBER OF CHARACTER
12714080001IN THE FIELD TO BEGIN ON:
13715060001FIELD SEPARATION CHARACTER [-t]:
14717060030ADDITIONS TO THE COMMAND LINE:

00140
01130
03011
04011
05011
07011
08011
09011
12120
13112
14152


"sort" allows you to sort the lines of a file.  This is the typical command
form for "sort".  A complete command form for "sort" is available through
the "sort" menu.  With this command form, you can specify the order that you
want the lines of the file in, and you can sort on a single field.

00Type the names of one or more files that you want to sort.  If you type
more than one file name, the files will be sorted together.
01Type the name of a file where you want "sort" to store the sorted file
or files.  Unless you give a file name here, "sort" will print the
output on your terminal screen.  If the file you name here exists, 
"sort" will overwrite the contents with its output. [-o]
03By default, "sort" orders the lines of files in ASCII Collating order.
If you select this field ,"sort" will use letter, digits,
spaces, and tabs to sort the lines of the file. You cannot use
this option with "n", "M" or "i". [-d]
04By default, "sort" orders the lines of files in ASCII collating order.
If you select this field, "sort" will order the lines of the file
according to the numeric value of numbers at the beginning of the lines.
For, the "ARITHMETIC VALUE" sort, "sort" organizes the lines according to
the first number it finds in the line. You cannot use this option
with "d", "M", or "i". [-n]
05By default, "sort" orders the lines of files in ASCII collating order.
Select this field, "sort" will order the lines of
the file according to the months of the year.  "sort" takes the first
three non-blank characters of the line, treats them as upper case, and
compares them as months.  JAN precedes FEB and all other months precede
DEC.  Lines with strings that are not valid months will be placed before
JAN.  You cannot use this option with "d", "n", or "i" options. [-M]
07This option compares only ASCII characters in the range of 040 -0176.
The characters in this range include alphanumeric characters and 
other printing characters normally found in text, e.g., comma, brackets,
and exclamation point. You cannot use this option with "d", "n", or 
"M".  [-i]
08This option reverses the printing order of the sorted lines.
You can use this option with "n","d", or "i" or "M" to reverse the 
sequence of lines. [-r]
09"sort" usually sorts all of the upper case then sorts all of the lower
case lines of a file.  With this option "sort" will sort
the upper case and the lower case together. [-f]
12"sort" normally begins sorting at the beginning of a line, but
you can choose a field that "sort" will begin sorting on.  "sort" 
will use the remainder of the line for sorting.  (See the complete command
form for a way to specify where "sort" should stop sorting.)  First type 
the number of fields that you want to skip, e.g., 1, then type the number 
of characters in the field that you want to start on, e.g., 2.  Separate
the two numbers with a dot (.). For example, 1.2 sorts the lines starting
in field 2 character 2.  All of the following result in sorting on the 
first character of the second field:  +1, +1., +1.0, and +1.1 .
13"sort" uses a blank as the default field separator.  You can designate
any single character, or string of characters,  as the field separator 
by typing it in this field. [-t]
14ASSIST appends anything that you type here to the command line.
You can use this field for "piping", "redirection", or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

03040507
04030507
05030407
07030405


030537n
040643n
050746n
070952n
081032n
091164n

003420001010
0103
0132if test -d $F01; then echo "$F01 IS A DIRECTORY";fi 
0305
0304yYnN
0405
0404yYnN
0505
0504yYnN
0705
0704yYnN
0804yYnN
0904yYnN
1208..
13011


001110
011010-o 
021110
030001y-dY-dnN
040001y-nY-nnN
050001y-MY-MnN
061110
070001y-iY-inN
080001y-rY-rnN
090001y-fY-fnN
101110
111110
121010+
131010-t''
141110




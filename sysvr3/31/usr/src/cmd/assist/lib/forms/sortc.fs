00000sort
00701060130NAME OF FILE(S):
01702060030NAME OF THE OUTPUT FILE [-o]:
02904020030VARIATIONS
03705060001MERGE THE LINES OF FILES [-m] (y/n):
04706060001CHECK THAT FILES ARE SORTED CORRECTLY [-c] (y/n):
05707060001PRINT ONLY UNIQUE LINES [-u] (y/n):
06909020030ORDERING RULES
07710060001"DICTIONARY" ORDER [-d] (y/n):
08711060001"ARITHMETIC VALUE" ORDER [-n] (y/n):
09712060001ORDER AS MONTHS OF THE YEAR [-M] (y/n):
10913060030ASCII ORDER IGNORING CHARACTERS
11714070001OUTSIDE THE RANGE OF 040 - 0176 [-i] (y/n):
12715060001REVERSE ORDER [-r] (y/n):
13716060001TREAT LOWER CASE AS UPPER CASE FOR COMPARISON [-f] (y/n):
14918020030SORTING ON A FIELD
15919060030BEGIN FIRST SORT - NUMBER OF FIELDS TO SKIP
16720080001AND NUMBER OF THE CHARACTER IN THE FIELD TO BEGIN ON:
17921060030END FIRST SORT - NUMBER OF FIELDS TO SKIP
18722080001AND NUMBER OF THE CHARACTER IN THE FIELD TO END BEFORE:
19923060030BEGIN SECOND SORT - NUMBER OF FIELDS TO SKIP
20724080001AND NUMBER OF THE CHARACTER IN THE FIELD TO BEGIN ON:
21925060030END SECOND SORT - NUMBER OF FIELDS TO SKIP
22726080001AND NUMBER OF CHARACTER IN THE FIELD TO END BEFORE:
23727060001IGNORE BLANKS AT THE BEGINNING OF FIELDS [-b] (y/n):
24728060001FIELD SEPARATION CHARACTER [-t]:
25930020030MEMORY AND BUFFER SPACE
26731060001NUMBER OF BYTES IN LINE BUFFER [-z]:
27732060001NUMBER OF KILOBYTES OF MAIN MEMORY [-y]:
28734060030ADDITIONS TO THE COMMAND LINE:

00280
01270
03001
04001
05001
07001
08001
09001
11001
12001
13001
16212
18222
20230
22242
23001
24202
26252
27262
28292


"sort" enables you to sort, merge, check the organization, and 
print only the unique lines of files.  It also allows you to sort
on a certain field or even two certain fields.  This command form
is the "complete" command form and enables you to create command lines
to perform all of these tasks. If you are interested in doing a 
simple sort of a file, you will find the "typical" sort command form
on the "sort" menu.

00Type the names of one or more files that you want to sort, merge,
or check.  If you give more than one file name, the files will 
be sorted or merged together.  You cannot give more than one 
file name with the "c" option.
01Type the name of a file where "sort" should store the sorted file
or files.  Unless you give a file name here, "sort" will print the
output on your terminal screen.  If the file you name here exists, 
ASSIST will stop you from overwriting the contents with the output 
of "sort".  You cannot specify an output file if you are checking 
a sorted file with the "c" option. [-o]
03With this option "sort" will merge files  that you have
previously sorted. With this option "sort" will not sort files.
To insure that "sort" functions properly when you merge files, 
you should use the "z" option to specify the length of the longest line
in the files.  You cannot use  "c", "d", "n", "M", "i", "y", or
any of the "SORTING ON A FIELD" arguments with this option. [-m]
04With this option, "sort" checks if your file is sorted properly.  
If it is not, you will get a message that says "disorder".
You can only use one file name with this option.  If you do not 
select any of the ORDERING options, "sort" tests for dictionary 
order.  When you use one of the ORDERING options, "sort" will 
test that the lines of the file are sorted according to that 
ordering rule, e.g.,sort -cn checks that the lines of the file 
are in arithmetic order.  You should use the "z" option with this 
option, to insure that "sort" will allocate space for the longest 
line in the file. You cannot use this option with "o", "m" or "y". [-c]
05If you use this option, "sort" prints only unique lines.
If there are multiple lines that are identical, "sort" with this
option will only print one of the lines.  You can use this 
option with the "m" option to  merge only one copy of identical 
lines, or with the "c" option to check that the lines in a sorted
file are unique. [-u]
07By default, "sort" orders the lines of files in ASCII Collating order.
If you select this option "sort" will use letter, digits, spaces, and 
tabs to sort the lines of the file. You cannot use this option with 
"m", "n", "M" or "i". [-d]
The "ORDERING RULES" can be applied globally to all sorting fields 
(see "SORTING..." below) or they can be used locally with a particular 
sorting field by stringing them after the sorting field, e.g. +1.0d or
+2.3r. In this command form, all "ORDERING RULES" will apply to all 
sorting fields. If you want them to apply locally to each sorting field,
use them as demonstrated here when you type you "sort" command line at 
shell level.
08By default, "sort" orders the lines of files in ASCII collating 
order.  With this option, "sort" will order the lines of the file
according to the numeric value of numbers at the beginning of the lines.
For, the "ARITHMETIC VALUE" sort, "sort" organizes the lines according to
the first number it finds in the line. You cannot use this option 
with "m", "d", "M", or "i". [-n]
The "ORDERING RULES" can be applied globally to all sorting fields 
(see "SORTING..." below) or they can be used locally with a particular 
sorting field by stringing them after the sorting field, e.g. +1.0d or
+2.3r. In this command form, all "ORDERING RULES" will apply to all 
sorting fields. If you want them to apply locally to each sorting 
field, use them as demonstrated here when you type you "sort" command 
line at shell level.
09By default, "sort" orders the lines of files in ASCII collating 
order.  If you use this option "sort" orders the lines of the file 
according to the months of the year.  "sort" takes the first three 
non-blank characters of the line, treats them as upper case, and 
compares them as months.  JAN precedes FEB and all other months precede 
DEC.  Lines with strings that are not valid months will be placed 
before JAN. You cannot use this option with "m", "d", "n", or "i".  [-M]
The "ORDERING RULES" can be applied globally to all sorting fields 
(see "SORTING..." below) or they can be used locally with a particular 
sorting field by stringing them after the sorting field, e.g. +1.0d or
+2.3r. In this command form, all "ORDERING RULES" will apply to all 
sorting fields. 
11This option compares only ASCII characters in the range of 040 - 0176.
The characters in this range include alphanumeric characters and
other printing characters normally found in text, e.g., comma, brackets,
and exclamation point.  This option is not compatible with "m", "d", "n",
or "M". [-i]
The "ORDERING RULES" can be applied globally to all sorting fields 
(see "SORTING..." below) or they can be used locally with a particular 
sorting field by stringing them after the sorting field, e.g. +1.0d or
+2.3r. In this command form, all "ORDERING RULES" will apply to all 
sorting fields. If you want them to apply locally to each sorting 
field, use them as demonstrated here when you type you "sort" command 
line at shell level.
12With this option "sort" reverses the order of the sorted lines.
Use this option with "n","d", or "i" or "M" to reverse the sequence
of lines. You cannot use this option with "m". [-r]
The "ORDERING RULES" can be applied globally to all sorting fields 
(see "SORTING..." below) or they can be used locally with a particular 
sorting field by stringing them after the sorting field, e.g. +1.0d or
+2.3r. In this command form, all "ORDERING RULES" will apply to all 
sorting fields. If you want them to apply locally to each sorting 
field, use them as demonstrated here when you type you "sort" command 
line at shell level.
13"sort" usually sorts all of the upper case then sorts all of the lower
case lines of a file.  With this option "sort" will sort the upper 
case and the lower case together. You cannot use this option with "m".
[-f]
The "ORDERING RULES" can be applied globally to all sorting fields 
(see "SORTING..." below) or they can be used locally with a particular 
sorting field by stringing them after the sorting field, e.g. +1.0d or
+2.3r. In this command form, all "ORDERING RULES" will apply to all 
sorting fields. If you want them to apply locally to each sorting 
field, use them as demonstrated here when you type you "sort" command 
line at shell level.
16"sort" normally begins sorting at the beginning of a line, but you
can choose a field that "sort" will begin sorting on.  First type the
number of fields that you want to skip, e.g., 1, then type the number
of characters in the field that you want to start on, e.g., 2.  Separate
the two numbers with a dot (.). For example, 1.2  sorts the lines starting
in field character 2.  All of the following result in sorting on the 
first character of the second field: +1, +1., +1.0, and +1.1.
18With this argument to "sort", you can specify the point where the sort 
is to end. Normally, "sort" compares lines of a file starting with the 
field that you specify an ending at the end of each line.  If you type 
1.4 in this field, "sort" will end the sort in the second field before 
character four.  If you specified sorting to start at 1.2 and end at 1.4
you would be sorting on the second and third characters of field two.
Note that all of the following denote ending before the first character
in the third field: -2, -2., -2.0, and -2.1. You cannot use this with "m".
20"sort" normally begins sorting at the beginning of a line, but you 
can choose one or more fields that "sort" will sort on.  You specify the 
second sorting field the same way you specify first.  First type the 
number of field that you want to skip, e.g., 1, then type the number of 
characters in the field that you want to start on, e.g., 4.  Separate 
the two numbers with a dot (.). For example, if the first sort field is 
1.2 to 1.4 and you specify the second field starting at 1.4, sort will 
sort the file according to field 2 characters 2 and 3 first, then it 
will arrange any lines that have identical characters 2 and 3 into order 
based on the characters in field 2 starting at character 4.  You cannot 
use this with "m".
22With this argument to "sort" you can specify the point where the second
sort field is to end.  Normally, "sort" uses the entire line to sort a
file.  If you type 1.5 in this field, "sort" will end the sort in  the 
second field before the fifth character in the field.  If your second 
sort field begins at 1.4 and ends at 1.5, "sort" will use the fourth 
character of field 2 as the second sort field. This is not compatible 
with "m".
23"sort" usually treats one or more blanks after a field
separator character as part of the next field.  With this
option, "sort" ignores leading blanks and starts at the 
first non-blank character after the field separator.
You can only use this option if you specify one or more
specific fields using the "SORTING ON A FIELD" arguments. 
This option cannot be used with "m". [-b]
24"sort" uses a blank as the default field separator.   You can designate
any single character, or string of characters, as the field separator
by typing it in this field. You cannot use this option with "m". [-t]
26Type the length of the longest line in the files you are checking "c"
or merging "m" files.  The length of a line is measured in bytes which
is equivalent to the number of characters in the line.  By specifying
the length of the longest line, you are assured that the line
buffer that "sort" uses will be large enough for every line in your file.
"sort" will terminate abnormally if the size of the buffer is too small 
to hold the lines of the file.  [-z]
27Type the number of kilobytes of main memory you need for your "sort".
"sort" begins with an amount of main memory that is the system default 
and adds memory as it is needed for sorting a file.  If your file
is small, you can reduce the impact of "sort" on your system's performance
by using only as much main memory as you need. [-y]
28ASSIST appends anything that you type here to the command line.
You can use this field for "piping", "redirection", or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0104
03040708091123242018162227
04010327
0708091103
0807091103
0907081103
1107080903
1203
1303
1603
1803
2003
2203
2303
2403

1816
201618
22201816
2320

030543n
040656n
050742n
071037n
081143n
091246n
111451n
121532n
131664n
232759n

003420001010
0105
0132if test -d $F01;then echo "$F01 IS A DIRECTORY";fi
0103
0304yYnN
0305
0404yYnN
0405
0504yYnN
0705
0704yYnN
0805
0804yYnN
0905
0904yYnN
1105
1104yYnN
1205
1204yYnN
1305
1304yYnN
1608...
1605
1619
1808...
1805
1816
1819
1832begin=$F16; end=$F18; diff=`echo "$begin $end" | /bin/awk '{ print ( $2 - $1 ) * 1000 }' -`; if test $diff -le 0; then echo "NUMBER FOR BEGIN MUST BE LESS THAN NUMBER FOR END"; fi
2008...
2019
2005
2016
2032begin=$F18; end=$F20; diff=`echo "$begin $end" | /bin/awk '{ print ( $2 - $1 ) * 1000 }' -`; if test $diff -lt 0; then echo "NUMBER FOR END MUST BE LESS THAN OR EQUAL TO THE NUMBER FOR BEGIN"; fi
2208..
2205
2216
2232begin=$F20; end=$F22; diff=`echo "$begin $end" | /bin/awk '{ print ( $2 - $1 ) * 1000 }' -`; if test $diff -le 0; then echo "NUMBER FOR BEGIN MUST BE LESS THAN NUMBER FOR END"; fi
2304yYnN
2316
2305
2405
24011
2608
2632zero=0; one=1; if [ $F26 -eq $one -o $F26 -eq $zero ]; then echo "THE LINE BUFFER MUST BE LONGER THAN 1 BYTE"; fi
2708
2732zero=0; one=1; if [ $F27 -eq $one -o $F27 -eq $zero ]; then echo "THE KILOBYTES OF MEMORY MUST BE GREATER THAN 1"; fi


001110
011110-o 
021110
030001y-mY-mnN
040001y-cY-cnN
050001y-uY-unN
061110
070001y-dY-dnN
080001y-nY-nnN
090001y-MY-MnN
101110
110001y-iY-inN
120001y-rY-rnN
130001y-fY-fnN
141110
151110
161010+
171110
181010-
191110
201010+
211110
221010-
230001y-bY-bnN
241010-t''
251110
261010-z
271010-y
281110




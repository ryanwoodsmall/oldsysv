#ident	"@(#)forms.files:comm.fs	1.4"
00000comm
00702060101NAME OF FIRST FILE:
01704060101NAME OF SECOND FILE:
02706060001PRINT LINES COMMON TO BOTH FILES [-12] (y/n):
03708060001PRINT LINES IN FIRST FILE BUT NOT IN SECOND [-23] (y/n):
04710060001PRINT LINES IN SECOND FILE BUT NOT IN FIRST [-13] (y/n):
05712060030ADDITIONS TO THE COMMAND LINE:

00020
01030
020122
030123
040123
05042

You are about to execute an interactive command.
The command will expect you to enter data from your terminal.
Type ^D when you are done entering input from your terminal.
This will terminate the command.00-

"comm" prints lines that are either unique or common
to two files.  The files must be pre-sorted in ASCII
collating order.  Use the "sort" command to do this.
By default, three columns of output are produced.
The left column contains those lines in the first
file that are not in the second.  The middle column
contains those lines that are in the second file,
but not the first.  The right column contains those
lines common to both files.

00Enter here the first file to be compared.  The file must be sorted
in ASCII collating order.  Use the "sort" command first, if the file
is not now sorted.
01Enter here the second file to be compared.  The file must be sorted
in ASCII collating order.  Use the "sort" command first, if the file
is not now sorted.
02Select this option if you want only those lines 
that are found in both files.  [-12]
03Select this option if you want only those lines that appear in
the first file, but are not in the second file.  [-23]
04Select this option if you want only those lines that appear in
the second file, but are not in the first file.  [-13]
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

020304
030204
040302


020652n
030863n
041063n

0001
0015-
0013
0032if [ ! "$F00" = "-" ]; then if [ "`/bin/sort -c $F00 2>&1`" ]; then echo "$F00 IS NOT SORTED.  USE THE 'SORT' COMMAND FIRST"; fi;fi
0101
0102
0113
0132if [ "`/bin/sort -c $F01 2>&1`" ]; then echo "$F01 IS NOT SORTED.  USE THE 'SORT' COMMAND FIRST"; fi
0205
0204YyNn
0305
0304YynN
0405
0404YyNn


001110
011110
021011y-12Y-12nN
031011y-23Y-23nN
041011y-13Y-13nN
051110




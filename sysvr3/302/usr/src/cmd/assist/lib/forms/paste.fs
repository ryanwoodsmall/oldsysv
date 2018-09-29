00000paste
00702100130NAMES OF INPUT FILES:
01704100001REDIRECT OUTPUT TO:
02706100001COLUMN SEPARATOR CHARACTER(S) [-d]:
03708100001PASTE INTO A SINGLE COLUMN [-s] (y/n):
04710100030ADDITIONS TO THE COMMAND LINE:

00030
01040
02022
03010
04042


The "paste" command will concatenate files horizontally.
For example, if you have two input files, the first line of output
will consist of the first line of file1, followed by a tab, followed
by the first line of file2, followed by a new-line character.
You can specify a list of separator characters other than a tab
on this command form.

00Enter the names of the files that you want to paste 
together horizontally.  If you enter only one file,
"paste" will have no effect.  "paste" will always
put a new-line character at the end of each line of 
the last file listed.  "-" may be used in place of
any filename to read a line from the standard input.
01If you type a filename here, the output of "paste" will go to the named
file.  ASSIST will warn you if the file you name already exists.  If it
exists, redirecting output to it will destroy the previous contents of 
the file.  (See the UNIX walkthru for a more complete discussion of
redirection.)
02By default, the new-line characters of each but the last file are 
replaced by a tab character.  That is, if you are pasting together 
3 files, the output will consist of the first line of file1, followed 
by a tab, followed by the first line of file2, followed by a tab, 
followed by the first line of file3 and a new line, etc. for each 
line of each file.  You may change the tab separator character.  
If you supply characters, they will be used circularly, one at a 
time -- when the list is exhausted it is reused.  The lines from the
file are always ended with a new-line.  For example, type ABC to 
replace the the first <TAB> with A, the second <TAB> with B, the 
third <TAB> with C, the fourth <TAB> with A, etc.  [-d]
03With the -s option, "paste" will attach all the lines
of the input file(s) to form one long, or some lines
longer than the original, depending on the separator
character(s) that you choose.  This is also known as 
a serial merging.  [-s]
04ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field,
you should use it with caution.



020646	
030849n

0015-
010114
0107
0103
0304yYnN


001110
011010> 
021010-d""	
031011y-sY-snN
041110




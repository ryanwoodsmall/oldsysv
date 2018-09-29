#ident	"@(#)forms.files:find.fs	1.8"
00000find
00901020030FILE SEARCH OPTIONS
01702060130NAMES OF PATHS FOR SEARCH OF TARGET FILE:
02703060001PROCESS DIRECTORY ENTRIES PRIOR TO DIRECTORY [-depth] (y/n):
03904060030RESTRICT SEARCH TO FILE SYSTEM CONTAINING CURRENT OR
04705080101SPECIFIED DIRECTORY [-mount] (y/n):
05907020030OUTPUT OPTIONS
06708060001PRINT NAME(S) OF FOUND FILE(S) [-print] (y/n):
07709060001NAME OF DEVICE TARGET FILE SHOULD BE WRITTEN ON [-cpio]:
08911020030PROPERTIES OF TARGET FILE
09712060001NAME OF TARGET FILE [-name]:
10713060001PERMISSION OF TARGET FILE [-perm]:
11714060001TYPE OF TARGET FILE [-type] (d/f/b/c/p):
12715060001NUMBER OF LINKS OF TARGET FILE [-links]:
13716060001OWNER OF TARGET FILE [-user]:
14717060001GROUP OWNER OF TARGET FILE [-group]:
15718060001SIZE OF TARGET FILE [-size]:
16719060001MOST RECENT ACCESS TIME OF TARGET FILE [-atime]:
17720060001MOST RECENT MODIFICATION TIME OF TARGET FILE [-mtime]:
18721060001MOST RECENT CHANGE TIME OF TARGET FILE [-ctime]:
19722060001FIND FILES NEWER THAN [-newer]:
20723060101TARGET FILE RESIDES ON THE LOCAL SYSTEM [-local] (y/n):
21924020030COMMAND EXECUTION
22725060001COMMAND TO EXECUTE ON FOUND FILE(S) [-exec]:
23926060000COMMAND TO EXECUTE ON FOUND FILE(S),
24727080001BUT FIRST PROMPT WITH "?" [-ok]:
25729020030ADDITIONS TO THE COMMAND LINE:

01010
02020
04030
06500
07510
09042
10050
11060
12070
13080
14090
15100
16110
17120
18130
19140
20150
22302
24312
25702


The purpose of "find" is to locate one or more files that satisfy 
conditions that the user specifies.  In the simplest case, the 
condition is that of having a specified name (e.g., assist.c).  
More complicated cases include, for example: the file must be
newer than a specified file, must have a specific permission
pattern, or must have been modified prior to a specified date.

Note that this command form reflects only some of the ways "find" 
is used.  Referring to the UNIX System V User Reference Manual,
the "find" command form has the following limitations:
1. No boolean operations on the conditions, only the default "and".
2. The -print, -exec, and -ok options can only occur at the end
   of the command line.  Thus, the -exec and -ok options cannot be
   used as conditions themselves, but only to perform operations
   on the found file(s).
3. Options cannot be repeated in the same command line.

01List here one or more path names.  "find" descends the 
directory hierarchy for each path name.  Make sure that
each path name is a directory with read and search permission.
02Under certain circumstances, it is useful when directory 
entries are processed prior to the directory itself.  An
example is when "find" is used with "cpio" to transfer
files that are contained in directories without
write permissions [-depth].
04Restricts the search to the file system containing the
directory specified, or if files were specified, the 
current directory [-mount].
06This option is almost always used.  It results in the found
file names being printed.  Without this option, "find"
finds files but nevers tells you it did.
This command form puts this option at the end of the command
line.  If it would be put prior to certain conditions in the
command line, all file names would be printed that satisfy the 
conditions that precede the -print option in the command line.
Usually, that is not what the user wants [-print].
07Write the target file in "cpio" format (5120 bytes) [-cpio].
09If you use characters that are special to the shell, 
such as [, ], ?, and *, it is advised to put single quotes 
around the file name [-name].
10Permissions are specified in the same manner as in
"chmod", absolute mode [see ASSIST command form].
For example, 755 signifies read, write, and execute
permissions for the owner, and read and execute 
permissions for group and other [-perm].
11This option allows you to find a file
that has a specified "type".  Available types and
their abbreviations are:
          d: directory
          f: plain file
          b: block special file
          c: character special file
          p: fifo (a.k.a. named pipe) [-type].
12Type -n for less than n, n for exactly n, and +n for more than n,
where "n" is a number.  For files, if the number of links is larger
than one, this means that the file is linked to another file.  
For directories, the number of links reflects the number of
subdirectories. [-links].
13Type name of existing user (consult /etc/passwd when in 
doubt).  This option allows you to find a file that
belongs to a specified user [-user].
14Type name of existing group (consult /etc/group when in
doubt).  This option allows you to find a file that
belongs to a specified group [-group].
15Type -n for less than n, n for exactly n, and +n for more than n.
n is a number.  If n is followed by the character "c",
it is interpreted in characters.  Otherwise n is
interpreted in blocks [-size].
16Type -n to find files which have been accessed less than n days ago,
n to find files which have been accessed exactly n days ago,
and +n to find files which have been accessed more than n days ago
[-atime].
17Type -n to find files which have been modified less than n days ago,
n to find files which have been modified exactly n days ago,
and +n to find files which have been modified more than n days ago
[-mtime].
18Type -n to find files which have been changed less than n days ago,
n to find files which have been changed exactly n days ago,
and +n to find files which have been changed more than n days ago
[-ctime].
19Type name of a file that has been modified earlier than
the target file [-newer].
20Checks if the target file physically resides on the local system.
The name of your local system is /bin/uname -s.
22Executes a command for those files that satisfy all specified
conditions.  

NOTE: Use the notation "{}" to pass these file names to the command.
      Escape spaces with ^C (otherwise ONE STRING ONLY error message
         is given).

[-exec].
24Executes a command for those files that satisfy all specified
conditions.  Execution takes place only after the user has responded
to a "?" sign by typing "y" 

NOTE: Use the notation "{}" to pass these file names to the command.
      Escape spaces with ^C (otherwise ONE STRING ONLY error message
         is given).

[-ok].
25ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020367n
040544n
060853n
202362n

013410001010
0132if test -f $F01 -o -x $F01; then exit; fi; echo " DIRECTORY CANNOT BE ACCCESSED ";
0204yYnN
0404yYnN
0435
0604yYnN
0703
0927[]?*'
100801234567
1104bcdpf
1208-+
1332/usr/bin/cut -f1,3 -d":" /etc/passwd | /usr/bin/egrep "$F13" >/dev/null;if test $? -eq 0; then exit;else echo "$F13 IS NOT A USER ON YOUR SYSTEM";fi
1432/usr/bin/cut -f1,3 -d":" /etc/group | /usr/bin/egrep "$F14" >/dev/null;if test $? -eq 0; then exit;else echo "$F14 IS NOT A GROUP ON YOUR SYSTEM";fi
1508-+Xc-c+c
1608-+
1708-+
1808-+
1902
2004yYnN


001110
011110
021011y-depth Y-depth nN
031110
041011y-mountY-mountnN
051010
061011y-print Y-print nN
071010-cpio 
081110
091010-name 
101010-perm 
111010-type 
121010-links 
131010-user 
141010-group 
151010-size 
161010-atime 
171010-mtime 
181010-ctime 
191010-newer 
201011y-local Y-local nN
211110
221010-exec  \;
231110
241010-ok  \;
251110




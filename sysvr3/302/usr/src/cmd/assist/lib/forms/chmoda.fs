#ident	"@(#)forms.files:chmoda.fs	1.3"
01000chmod
00701020130NAME OF FILE(S) WHOSE MODE IS TO BE CHANGED:
01703020101GIVE OWNER PERMISSION TO READ (y/n):
02704260101WRITE (y/n):
03705240101EXECUTE (y/n):
04707020101GIVE GROUP PERMISSION TO READ (y/n):
05708260101WRITE (y/n):
06709240101EXECUTE (y/n):
07711020101GIVE OTHERS PERMISSION TO READ (y/n):
08712270101WRITE (y/n):
09713250101EXECUTE (y/n):
10715020101SET USER ID ON EXECUTION  (y/n):
11716020101SET GROUP ID ON EXECUTION / ENABLE MANDATORY LOCKING (y/n):
12717020101SET STICKY BIT            (y/n):

00010
01020
02030
03040
04050
05060
06070
07080
08090
09100
10110
11122
12130


In the absolute mode, you directly specify the permission pattern
as a string of three or four octal numbers.  These numbers stand for a
12 or 16-bit string, where each bit corresponds to a particular
permission.  For example, the rightmost bit stands for execute
permission for "others".

NOTE: The owner of a file is the user id of the person who created 
the file, unless file ownership has been transferred with the 
"chown" command.  The group of a file is the group id of the person
who created the file, unless file ownership has been transferred 
with the "chgrp" command.  "Other" refers to all users on a system 
other than the owner and the group.

00List file names here.  You must own these files.
01Read permission allows any operations on a file that
involve looking at its contents.
02Write permission allows changing the contents of a file.
If the argument is a directory, write permission is
interpreted as permission to move or copy files to the 
directory or remove files from the directory.
03Execute permission is required for any file you want to execute.
If the argument is a directory, execute permission is
interpreted as the permission to access the directory.
In addition, execute permission is required to copy or move
files to the directory and remove files from the directory.
04Read permission allows any operations on a file that
involve looking at its contents.
05Write permission allows changing the contents of a file.
If the argument is a directory, write permission is
interpreted as permission to move or copy files to the 
directory or remove files from the directory.
06Execute permission is required for any file you want to execute.
If the argument is a directory, execute permission is
interpreted as the permission to access the directory.
In addition, execute permission is required to copy or move
files to the directory and remove files from the directory.
07Read permission allows any operations on a file that
involve looking at its contents.
08Write permission allows changing the contents of a file.
If the argument is a directory, write permission is
interpreted as permission to move or copy files to the 
directory or remove files from the directory.
09Execute permission is required for any file you want to execute.
If the argument is a directory, execute permission is
interpreted as the permission to access the directory.
In addition, execute permission is required to copy or move
files to the directory and remove files from the directory.
10[see UNIX System V User's Reference Manual]
11The effect of this option depends on whether you 
have selected group execution permission.  
If you have, then group ID will be set on execution.  
If you have not, then mandatory locking will be enabled.
12Even though the "chmod" command will not complain, you can 
select this option only if you are a super user.



010339n
020439n
030539n
040739n
050839n
060939n
071140n
081240n
091340n
101535n
111662n
121735n

0002
0009
0117yYnN
0104yYnN
0204yYnN
0304yYnN
0404yYnN
0504yYnN
0604yYnN
0704yYnN
0804yYnN
0904yYnN
1004yYnN
1104yYnN
1204yYnN


001110
011011yXYXnN
021011yXYXnN
031011yXYXnN
041011yXYXnN
051011yXYXnN
061011yXYXnN
071011yXYXnN
081011yXYXnN
091011yXYXnN
101011yXYXnN
111011yXYXnN
121011yXYXnN




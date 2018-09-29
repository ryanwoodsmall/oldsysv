#ident	"@(#)forms.files:chmods.fs	1.4"
00000chmod 
00702060130NAME OF FILES WHOSE PERMISSION YOU WANT TO CHANGE:
01704060101WHICH USER'S PERMISSIONS DO YOU WANT TO CHANGE: OWNER  [u] (y/n):
02705540101GROUP  [g] (y/n):
03706540101OTHERS [o] (y/n):
04708060101ACTION TAKEN WITH PERMISSION(S): ADD    [+] (y/n):
05709390101DELETE [-] (y/n):
06710390101SET    [=] (y/n):
07712060101PERMISSION(S) ACTED ON: READ              [r] (y/n):
08713300101WRITE             [w] (y/n):
09714300101EXECUTE           [x] (y/n):
10715300101SET USER ID       [s] (y/n):
11716300101SET STICKY BIT    [t] (y/n):
12717300101MANDATORY LOCKING [l] (y/n):

00130
01011u
02021g
03031o
04041+
05051-
06061=
07071r
08081w
09091x
10101s
11111t
12121l


In the symbolic mode, the mode is specified by a string that has a
"who" part, a "how" part and a "what" part.  In the "who" part,
you specify whose permission is to be changed. In the "how" part,
you specify how the permission is to be changed.  In the "what"
part, you specify which permission(s) are to be changed.  Single-letter
abbreviations are used. "chmod" allows you to have more than one
mode string, separated by commas.  This is not implemented in the form.

00List file names here.  You must own these files.
01The owner of a file is the user id of the person who 
created the file, unless file ownership has been transferred 
with the "chown" command.
If you do not select one of these three, "all" will be assumed [u].
02The group of a file is the group id of the person who 
created the file, unless file ownership has been transferred 
with the "chgrp" command.
If you do not select one of these three, "all" will be assumed [g].
03"Other" refers to all users on a system other than the owner and 
the group.
If you do not select one of these three, "all" will be assumed [o].
04If a file does not have the specified permission,
this permission is added. If a file does not
have the specified permission, nothing happens  [+].
05If a file has the specified permission(s), this permission is
taken away; otherwise, nothing happens.
Other permissions are not changed [-].
06The file is given the specified permission(s) for the 
specified user, regardless of what the current permission is.
For example, if you set user read permission, then the 
file will not have user write and execute permission.
However, permissions for group and other are unaffected[=].
07Read permission allows any operations on a file that
involve looking at its contents [r].
NOTE: the "set" [=] option may affect this permission,
even if not selected.
08Write permission allows changing the contents of a file.
If the argument is a directory, write permission is
interpreted as permission to move or copy files to the 
directory or remove files from the directory [w].
NOTE: the "set" [=] option may affect this permission,
even if not selected.
09Execute permission is required for any file you want to execute.
If the argument is a directory, execute permission is
interpreted as the permission to access the directory.
In addition, execute permission is required to copy or move
files to the directory and remove files from the directory [x].
NOTE: the "set" [=] option may affect this permission,
even if not selected.
10See UNIX System V User's Reference Manual [s].
NOTE: the "set" [=] option may affect this permission,
even if not selected.
11See UNIX System V User's Reference Manual [t].
NOTE: the "set" [=] option may affect this permission,
even if not selected.
12See UNIX System V User's Reference Manual [l].
NOTE: the "set" [=] option may affect this permission,
even if not selected.

040506
050406
060405


010472n
020572n
030672n
040857n
050957n
061057n
071259n
081359n
091459n
101559n
111659n
121759n

0002
0009
0104yYnN
0204yYnN
0304yYnN
0404yYnN
0405
0431
0504yYnN
0505
0531
0604yYnN
0605
0631
0704yYnN
0731
0804yYnN
0831
0904yYnN
0931
1004yYnN
1031
1104yYnN
1131
1231
1204yYnN


001110
010001yuYunN
020001ygYgnN
030001yoYonN
040001y+Y+nN
050001y-Y-nN
060001y=Y=nN
070001yrYrnN
080001ywYwnN
090001yxYxnN
100001ysYsnN
110001ytYtnN
120001ylYlnN

040506
050406
060405
071208091011
081207091011
091207081011
101207080911
111207080910
120708091011



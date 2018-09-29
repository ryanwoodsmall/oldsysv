#ident	"@(#)forms.files:umask.fs	1.2"
01100umask
00701060101PRESERVE PERMISSION OF OWNER TO READ:
01702370101WRITE:
02703350101EXECUTE:
03705060101PRESERVE PERMISSION OF GROUP TO READ:
04706370101WRITE:
05707350101EXECUTE:
06709050101PRESERVE PERMISSION OF OTHERS TO READ:
07710370101WRITE:
08711350101EXECUTE:

00000
01000
02000
03000
04000
05000
06000
07000
08000


"umask" is used to remove permissions when a new file is created.  
Like "chmod", it uses an octal code to specify the permissions.
For example, 022 removes write permission for "group" and "others".
The actual permissions a file obtains depends both on the
permissions given to it by the program that generated it (e.g., 
cc creates an "a.out" file with execute permission) and on the 
file-creation mask as specified by "umask".

"Creation" has a slightly different meaning from what one would 
expect.  That is, creating a new file by using the "mv" or "cp" 
commands is not "creation";  however, creating a new file by using 
"ed", "vi", or "cat >" is "creation".  In the former case, the 
permissions of the new file match those of the moved or copied file, 
regardless of the file-creation mask; in the latter case, the 
permission is as specified by the file-creation mask.

Note that, like "cd", execution of "umask" in ASSIST stays in
effect only as long as you stay in ASSIST.

00Read permission allows any operations on a file that
involve looking at its contents.
The "owner" is the ID of the user who created the file.
01Write permission allows changing the contents of a file.
For directories, write permission is interpreted as permission 
to move or copy files to the directory or remove files from 
the directory.
The "owner" is the ID of the user who created the file.
02Execute permission is required for any file you want to 
execute.  For directories, execute permission is interpreted 
as the permission to access the directory.  In addition, 
execute permission is required to copy or move files to the 
directory and remove files from the directory.
The "owner" is the ID of the user who created the file.
03Read permission allows any operations on a file that
involve looking at its contents.
The "group" is the group ID of the user who created the file.
04Write permission allows changing the contents of a file.
For directories, write permission is interpreted as permission 
to move or copy files to the directory or remove files from 
the directory.
The "group" is the group ID of the user who created the file.
05Execute permission is required for any file you want to 
execute.  For directories, execute permission is interpreted 
as the permission to access the directory.  In addition, 
execute permission is required to copy or move files to the 
directory and remove files from the directory.
The "group" is the group ID of the user who created the file.
06Read permission allows any operations on a file that
involve looking at its contents.
"Other" refers to users who have an individual and
group ID different from that of th user who created the file.
07Write permission allows changing the contents of a file.
For directories, write permission is interpreted as permission 
to move or copy files to the directory or remove files from 
the directory.
"Other" refers to users who have an individual and
group ID different from that of th user who created the file.
08Execute permission is required for any file you want to 
execute.  For directories, execute permission is interpreted 
as the permission to access the directory.  In addition, 
execute permission is required to copy or move files to the 
directory and remove files from the directory.
"Other" refers to users who have an individual and
group ID different from that of th user who created the file.



000144y
010244y
020344y
030544y
040644y
050744y
060944y
071044y
081144y

0004yYnN
0104yYnN
0204yYnN
0304yYnN
0404yYnN
0504yYnN
0604yYnN
0704yYnN
0804yYnN


001011nXNXyY
011011nXNXyY
021011nXNXyY
031011nXNXyY
041011nXNXyY
051011nXNXyY
061011nXNXyY
071011nXNXyY
081011nXNXyY




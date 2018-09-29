00000rm
00701060030NAME OF FILE(S):
01703060001REMOVE DIRECTORIES AND THE CONTENTS OF THE DIRECTORIES [-r] (y/n):
02704060030NAME OF DIRECTORY(S):
03906020000OPTIONS FOR FILES AND DIRECTORIES
04707060001PROMPT FOR FILE(S) WITH INCORRECT PERMISSIONS [-f] (y/n):
05708060001PROMPT BEFORE REMOVING EACH FILE AND/OR DIRECTORY [-i] (y/n):
06710060030ADDITIONS TO THE COMMAND LINE:

00040
01001r
02050
04001f
05001i
06062

This is the interactive option to "rm".
If you want to exit the "rm" command 
type ^D.05Yy

"rm" removes files.  To remove directories,
and the files and subdirectories
in the directories, you must use the -r
option.  You can use the options in the 
"OPTIONS FOR FILES AND DIRECTORIES" section
if you're removing files or directories.

00Type the name or names of files that you want to remove.
"rm" requires either a file or directory name on the
command line.
01With this option, "rm" removes all the files and sub-directories 
within a directory.You must respond "y" to this field if you want 
to remove a directory. "rm" will remove the contents of the directory 
then it will remove the directory itself. [-r]
02Type the name or names of directories that you want to remove.  
You must use the -r option if you want to remove directories.  
"rm" requires either a file or directory name on the command line.
04If you respond "y" to this caption, "rm" 
removes files and directories without telling you
that you do not have write permission.  Normally,
if you do not have write permission, then "rm" asks
you if you want to remove the file. This option makes
no sense with the -i option, since the "i" option
prompts you for a response. [-f]
05If you replace the "n" with a "y", "rm" will
prompt you for every file or directory that it 
encounters.  If you type "y" after the file
and directories, "rm" will remove them.  If you
type "n", "rm" will not remove them.  You can 
not use this option with the -f option, because
the -f option suppresses prompting. [-i]
06ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field, 
you should use it with caution.

0405
0504

0201

010373n
040764y
050868n

0014
0013
0002
0031
003420001010
0119
0104yYnN
0210
0216
0231
023400001010
0405
0417yYnN
0404nNyY
0505
0517yYnN
0504yYnN


001110
010001y-rY-rnN
021110
040001n-fN-fyY
050001y-iY-inN
061110

0002
0200



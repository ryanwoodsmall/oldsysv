00000rmdir
00701060130DIRECTORY NAME(S):
01703060001REMOVE EMPTY PARENT DIRECTORIES  [-p] (y/n):
02705060001SILENCE MESSAGES ABOUT REMOVING PARENT DIRECTORIES [-s] (y/n):
03707060030ADDITIONS TO THE COMMAND LINE:

00010
01000
02000
03112


"rmdir" stands for remove directory.  "rmdir" will
remove only empty directories.  To use "rmdir" you 
must own a directory and have write permission for it.

00Type the names of all the directories you want to remove. 
You must own the directories and they must be empty for you
to delete them.  See the "rm" command form if you want to 
remove directories that have files and subdirectories.
01Normally, "rmdir" only removes the directory that you give as
an argument.  With this option, "rmdir" will also remove parent
directories that become empty as a result of removing the 
directory.  "rmdir" will give you messages about the directories
that were removed [-p].
02This option silences that messages you get when you delete
a directory and its parent directories with the "p" option [-s].  
03ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field,
you should use it with caution.


0201

010351n
020569n

0010
0025
0032string=$F00; curdir=`pwd`; if [ ! -w `/bin/dirname $string` ]; then echo "YOU NEED WRITE PERMISSION IN `/bin/dirname $F00` TO REMOVE `/bin/basename $F00`"; else if [ "`ls $F00`" ]; then echo "$F00 CONTAINS FILES AND/OR DIRECTORIES. YOU MUST REMOVE THEM FIRST."; else case "$string" in $curdir) ;; `/bin/basename $curdir`) ;; "../`/bin/basename $curdir`") ;; \.) ;; \.\.) echo "$F00 MUST BE EMPTY BEFORE YOU CAN REMOVE IT." ; exit ;; *) exit ;; esac; echo "YOU CANNOT REMOVE YOUR CURRENT DIRECTORY"; fi; fi
 5
003400101010
0104yYnN
0119
0135
0204yYnN
0216
0235


001110
011011y-pY-pnN
021011y-sY-snN
031110




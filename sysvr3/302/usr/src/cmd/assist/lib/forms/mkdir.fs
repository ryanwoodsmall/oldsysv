00000mkdir
00701060130NAME OF DIRECTORY(S):
01703060001MODE TO USE FOR NEW DIRECTORIES [-m]:
02705060001CREATE ALL NON-EXISTING PARENT DIRECTORIES FIRST [-p] (y/n):
03707060030ADDITIONS TO THE COMMAND LINE:

00010
01000
02000
03022


The mkdir command creates specified directories.  You must have write
permission in the parent directory.  

00List the names of the directories for mkdir to create.  You must
supply at least one directory name.
01Use this option if you want to specify the mode for the new
directory you are creating.  The modes you may specify are
read, write, and execute.  You may set these modes for the
user, group, and other permissions.  Octal notation is
used to set the mode.  The following list shows the octal
numbers and their representation:
        0       no permissions          4       read
        1       exec                    5       read, exec
        2       write                   6       read, write
        3       write, exec             7       read, write, exec
For example, if you want to allow only read permission for everybody,
you would type -m444.  The default mode is specified as 777 which gives 
user, group, and others read, write, and execute permission.  Refer to the
"chmod" and "umask" manual pages for a more detailed description
of how to set the mode. [-m]
02This option creates all the non-existing  parent directories
before it creates the new directory. [-p]
03MUSE appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
MUSE does not perform any error checking on this field,
you should use it with caution.



010344expr 0777 - `umask`
020567n

0032if [ -f $F00 -o -d $F00 ] ; then echo "$F00 ALREADY EXISTS"; exit; fi; if [  ! -w `dirname $F00` -a -d `dirname $F00` ] ; then echo "`dirname $F00` DOES NOT HAVE WRITE PERMISSION"; exit; fi; if [ ! -d `dirname $F00` -a  \( "$F02" = "n" -o "$F02" = "N" \) ] ; then echo "`dirname $F00` DOES NOT EXIST"; exit; fi;
010801234567
0135
0204yYnN
0235


001110
011010-mexpr 0777 - `umask`
021011y-pY-pnN
031110




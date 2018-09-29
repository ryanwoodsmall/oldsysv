00000chown
00701060130FILE AND/OR DIRECTORY NAME(S):
01003060030NEW OWNER'S LOGIN NAME OR
02704080130NUMERIC USER ID:
03706060030ADDITIONS TO THE COMMAND LINE:

00020
02010
03032


"chown" changes the owner of files and
directories.  You name the new owner of 
file or directory using the person's 
login name or numeric user ID.  You must
own the file or directory, or you must be 
super user to use "chown".

00Type the names of files and directories for which you would like to 
designate a new owner.  You must be the current owner of 
the files and directories in order to designate a new owner.
02Type the login name  or numeric user ID of the 
person you want to designate as the new owner
of the files or directories.  A numeric user
ID is a numeric representation of a user's login
name.  The numeric user IDs for people on your system
are listed in the third field of the /etc/passwd file.
03ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.




0014
0032if [ ! -f $F00 -a ! -d  $F00 ];then echo "$F00 DOES NOT EXIST"; exit; fi; name=`id|/bin/sed -e 's/^uid\=[0-9]*(//' | /bin/sed -e 's/).*$//' `; ls -ld $F00 | /bin/grep " $name " > /dev/null; if test $? -eq 0; then exit; else id|/bin/grep "uid=0" > /dev/null; if test $?  -ne 0; then echo "YOU MUST OWN $F00 OR BE SUPERUSER"; fi; fi;
0218-
0232/usr/bin/cut -f1,3 -d":" /etc/passwd | /usr/bin/egrep "$F02" 1>/dev/null 2>&1;if test $? -eq 0; then exit;else echo "$F02 IS NOT A USER ON YOUR SYSTEM";fi


001110
011110
021110
031110




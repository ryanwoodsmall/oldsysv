00000su
00701060001USER LOGIN NAME:
01703060030ARGUMENTS FOR LOGIN SHELL:
02705060001INCLUDE THE LOGIN ENVIRONMENT (y/n):
03707060030ADDITIONS TO THE COMMAND LINE:

00010root
01020
02011-
03032

This is an interactive command.  In order to return to ASSIST
when you are done, type control-d (^D).

"su" allows you to re-login in, without terminating
your current session.  This is useful if you want to
log in as a different user, execute a few commands,
and then return to your current session.  In effect,
this allows you to change your identity to a different
login name.

00Enter here the user login id which you want to become.
If you do not enter a value, the default is "root."
01Anything you place here will be passed to the program invoked as your
shell.  This program is specified in your password entry in the
/etc/passwd file.  You might use this field to pass a command to be
executed (for example, -c command_name), or to indicate that a
restricted shell should be run (-r).  See the "sh" command form or
manual page for other arguments.  In order to use this field, you must
specify a value in the first field.  This field is not validated by 
ASSIST.
02By default, you will take on the new user login id, but will remain
in the same environment.  That is, shell environment variables like
$HOME, $PATH, etc. will not be changed.  Enter a "y" in this field
if you want your environment set as if you had just logged in.  
(The system /etc/profile and the user's .profile are executed.) [-]
03ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.


0100

020543n

0032if [ ! "`/bin/grep \"^$F00:\" /etc/passwd`" ]; then echo "$F00 IS NOT A VALID USER LOGIN ID ON YOUR SYSTEM"; fi
0019
0116
0204yYnN


001010root
011110
020001y-Y-nN
031110




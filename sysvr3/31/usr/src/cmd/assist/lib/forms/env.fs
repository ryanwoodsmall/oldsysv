#ident	"@(#)forms.files:env.fs	1.4"
00000env
00701060030COMMAND TO BE EXECUTED:
01703060030VARIABLE=VALUE PAIRS:
02705060001INCLUDE INHERITED ENVIRONMENT [-] (y/n):
03707060030ADDITIONS TO THE COMMAND LINE:

00030-
01022
02001-
03042


"env" is used in two different ways.  First, it can
be used to list all of your environment variables
and their current values (in which case, leave the
COMMAND TO BE EXECUTED field empty).  The second use 
of "env" is to execute a command with new values for your
environment variables.

00Enter here the name of the command you want executed, along
with any arguments that are expected by the command.
No error checking is performed on this field by ASSIST, since
the VARIABLE=VALUE pairs that you enter may alter your
environment in ways that ASSIST cannot predict.
01If you wish to modify the current environment before executing the
command, enter variables and their values in the form:
  variable=value
You may enter multiple variable=value pairs.
02By default, your current environment is passed on to the command,
modified by the "VARIABLE/VALUE PAIRS," if supplied.  Enter a "n"
here if you want to ignore the inherited environment, and only
use the variables supplied on this command form. [-]
03ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020547y

0132if [ "`echo \"$F01\" | /bin/grep -v = `" ];then echo "MUST BE IN FORM VARIABLE=VALUE";fi
0204nNyY


001110
011110
020001n-N-yY
031110




#ident	"@(#)forms.files:nohup.fs	1.2"
00000nohup
00703060130COMMAND:
01705060030ADDITIONS TO THE COMMAND LINE:

00010
01022


"nohup" executes the commands, and prevents them
from terminating if it receives a "hangup" or "quit"
signal.  You might use this command if you wanted to
execute a command and log off before the command
completed.  Normally, when you log off, any
currently running commands are terminated.

00Enter in this field the command that you want to run.  Include
all arguments to the command, as if you were entering it on
the command line.  Normally, when a "break" or "hangup" occurs
while a command is running, that command is terminated.  The
command entered with "nohup" will continue to run under such
circumstances.  ASSIST does not validate this field.
01ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.






001110
011110




#ident	"@(#)forms.files:Batch.fs	1.2"
400BATCH
00001140000BATCH, BACKGROUND, AND EXECUTION CONTROL
01703110101..... Terminate a process
02704120101.... Run a command immune to hangups and quits
03705090101....... Report status of currently executing commands
04706090101....... Execute commands
05008080000MORE COMMANDS FOR BATCH, BACKGROUND, AND EXECUTION CONTROL
06710090101....... Execute commands at a later time
07711120101.... Execute in the background when system load permits
08712110101..... Run a command in the background at low priority

The Batch, Background, and Execution Control menu lists
commands that control many commands at one time, give
process status, and terminate processes.

01"kill" terminates processes.  It does this by
sending a signal number to the specified processes.
By default, "kill" sends signal number 15 to kill
a process.
02"nohup" executes the commands, and prevents them
from terminating if it receives a "hangup" or "quit"
signal.  You might use this command if you wanted to
execute a command and log off before the command
completed.  Normally, when you log off, any
currently running commands are terminated.
03"ps" reports the status of processes that are
running on your system.  Without options, "ps"
prints information on processes associated with
your terminal.  With options, "ps" can report on
the status of processes that were started by other
users on your system.
04"sh" is the UNIX System command interpreter.  "sh"
is used to execute one or more commands from a file,
the command line, or your terminal.
06"at" causes commands to be executed at a later time.
"at" reads the commands that it will execute from
standard input.  By default, the output is mailed
to the user.  The following sequence is an example
of "at" usage:


$ at 5:00pm                     <----- User enters "at" command
echo "Wake up; quitting time"   <----- User types a command
^D                              <----- User types ^D to end standard input


At 5:00 pm, when the user reads his/her mail,
s/he will get the message "Wake up; quitting time."
07"batch" reads commands from standard input and
executes those commands at a later time when
system load permits.  The following sequence is
an example: 


    $batch
    sort hugefile > outfile
    ^D                      <---- (Type control-d to end standard input)
08"nice" executes a command at a lower CPU scheduling
priority.  As a courtesy to other system users, you
may want to use this command when your process uses
a lot of processor resources.  You change the
priority by specifying an increment from 1-19; the
higher the number, the lower the priority.  If you
don't specify an increment, 10 is assumed.  A super-
user may raise the priority by putting a "-" before
the increment, e.g., --9.  

010306kill
020406nohup
030506ps
040606sh
061006at
071106batch
081206nice

01kill
02nohup
03ps
04sh
06
07
08


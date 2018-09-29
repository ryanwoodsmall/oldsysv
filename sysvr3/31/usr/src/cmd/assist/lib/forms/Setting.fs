#ident	"@(#)forms.files:Setting.fs	1.3"
400SET ENVIRONMENTS
00001170000SET UP USER AND TERMINAL ENVIRONMENTS
01703110101..... Set environment for command execution
02704120101.... Set or report permissions for others to "write" to you
03705140101.. Change login password
04706120101.... Report or set options for a terminal
05707100101...... Become super user or another user
06708130101... Set code that changes permissions of a file when created
07010110000MORE COMMANDS FOR SETTING UP USER AND TERMINAL ENVIRONMENTS
08712140101...Put variable values in your environment
09713130101... Sign on
10714120101.... Set tab stops on your terminal

The Set Up of User and Terminal Environments menu
lists commands that help you control your terminal,
establish or change your login password, and set
your environment for creating files.

01"env" is used in two different ways.  First, it can
be used to list all of your environment variables
and their current values.  The second use of "env"
is to execute a command with new values for your
environment variables.
02"mesg" can be used in two ways.  First, you can
use it to prevent or allow users to "write" to
your terminal.  Second, it can tell you whether
users are currently able to "write" to your
terminal.
03"passwd" changes your password or puts a new
password in place for your login name.  You will
be prompted for your current password, before it
prompts you for your new password.  Passwords must
have the following characteristics:
 
  1. must be at least 6 characters long
  2. must contain at least 2 alphabetic characters
     and at least 1 numeric or special character
  3. must differ from your login name

Ordinary users may change only the password which
corresponds to their login name.  If you have super
user permission for your system, you may change any
password.  In the event you forget your password,
the system administrator can give you access to your
login again.
04"stty" sets many of your terminal options.  Use
"stty" without arguments to see the current set-
tings of certain options, such as the baud rate
and the current erase characters.  Use "stty -a"
to see the current settings of all options.  You
can change an option setting using "stty" with
the option name and a new value.
05"su" allows you to re-login in, without terminating
your current session.  This is useful if you want to
log in as a different user, execute a few commands,
and then return to your current session.  In effect,
this allows you to change your identity to a different
login name.
06"umask" is used to remove permissions when a new
file is created.  It uses the same "absolute" mode
as the "chmod" command.  Thus, 022 will remove
write permission for "group" and "others".
08"export" has two uses.  First, you can use it to
list the current value of all environment variables
that you have marked for export in the current shell.
Second, you can use it to mark a variable for export.
When a variable is marked for export, commands issued
from the shell will have access to the value of the
variable.
09"login" is used at the beginning of each terminal
session and allows you to identify yourself to the
system.  It may be invoked as a command or by the
system when a connection is first established.  If
you do not complete the login successfully within
a certain period of time (e.g., one minute), you
are likely to be silently disconnected.
10"tabs" can be used by you to set the tab stops on
your terminal.  A number of different tab settings
are pre-defined.  You may also define your own tab
settings.  In order to use "tabs", your terminal
must have the ability to remotely set its hardware
tabs.

010307env
020407mesg
030507passwd
040607stty
050707su
060807umask
081207export
091307login
101407tabs

01env
02mesg
03passwd
04stty
05su
06umask
08
09
10


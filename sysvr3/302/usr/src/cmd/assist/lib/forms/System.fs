400INFORMATION
00001130000SYSTEM, TERMINAL, AND GENERAL INFORMATION
01702250030.. Brief introduction to "assist"
02703230030.... Brief introduction to the UNIX system 
03704110101................ Show the calendar for a specified year
04705120101............... Show and set the date and time
05706120101............... Write arguments to the standard output
06707110101................ Report or set environment for command execution
07708100101................. Print user and group IDs names
08709100101................. List contents of directory
09710100101................. Report process status
10711110101................ Print name of working directory
11712120101............... Report or set options for terminal
12713130101.............. Show name of current UNIX system
13714110101................ Show who is on the system
14016090000MORE COMMANDS FOR SYSTEM, TERMINAL, AND GENERAL INFORMATION
15717160030........... Show today's entries for the calendar file
16718110101................ Show the pathname of user's terminal
17719140101............. List "uucp" names of known systems

The System, Terminal, and General Information menu
lists commands that report information about your
system and your login and terminal evironments.
You can find out the current date; get user, group
and, terminal names; and list who is currently on
the system.

01The "assist" walkthru provides an interactive demonstration of 
"assist."  The walkthru covers the major concepts and commands
you will use in "assist" 
02The Unix system walkthrus give you interactives demonstrations of key 
Unix system concepts and command.  The walkthrus include discussions of 
files, directories, command execution, and input and output 
redirection.  
03"cal" prints a calendar on your terminal.  If you
specify neither a month nor a year, "cal" will print
a calendar for the current month in the current year.
04"date" prints the day of the week, date, and time.
If you would like to print this information in a
different way, "date" enables you to change the
format.  If you have super user permissions for
your computer system, you can also change the date
using the "date" command.
05"echo" prints strings on your terminal.  One use
of "echo" is to find out the current values of your
environment variables.  For example, to find out your
current login directory, you could enter "echo $HOME".
06"env" is used in two different ways.  First, it can
be used to list all of your environment variables
and their current values.  The second use of "env"
is to execute a command with new values for your
environment variables.
07"id" returns your user id and group id.  These ids
are shown in both their mnemonic and numeric forms.
"id" requires no input.
08
"ls" lists contents of a directory or directories,
and can give detailed information about each file
listed.

The "ls" menu lists two versions of the "ls" command
form; a short "typical" use of "ls", and a complete
version that incorporates all of the options.
09"ps" reports the status of processes that are
running on your system.  Without options, "ps"
prints information on processes associated with
your terminal.  With options, "ps" can report on
the status of processes that were started by other
users on your system.
10"pwd" prints the path name of the current working
directory.
11"stty" sets many of your terminal options.  Use
"stty" without arguments to see the current set-
tings of certain options, such as the baud rate
and the current erase characters.  Use "stty -a"
to see the current settings of all options.  You
can change an option setting using "stty" with
the option name and a new value.
12"uname" prints the name of the UNIX system you are
using.  Besides determining the system name, you
can retrieve other information such as the name
that the system is known by to a communication
network, system release and version, and machine
hardware name.
13"who" gives you information about your system and
the users on your system.  By default, "who" prints
the names of users who are currently logged in to
your system, their terminal numbers and the time
that they logged in.  You can get information on the
system or you can change user information you get.
15"calendar" provides you with a simple reminder system.
"calendar" consults a file you create and prints out
all lines from the file that contain either today's
or tomorrow's date anywhere in the line.  The file
name must also be "calendar" and must be in the
current directory.
16"tty" prints the path name of your terminal.  This
includes the terminal's file system name and your
terminal number.
17"uuname" lists the "uucp" names of systems known
to your system.  If a system name is printed by
"uuname", you can send "mail" to users on that
system, use "cu" to call up that system, and
"uucp" to copy files to the system.

010207assist assistwalk
020307assist unixwalk
030407cal
040507date
050607echo
060707env
070807id
080907ls
091007ps
101107pwd
111207stty
121307uname
131407who
151707calendar
161807tty
171907uuname

01assistwalk
02unixwalk
03cal
04date
05echo
06env
07id
08ls
09ps
10pwd
11stty
12uname
13who
15
16
17


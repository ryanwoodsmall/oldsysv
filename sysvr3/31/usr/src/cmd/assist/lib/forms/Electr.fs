400ELECTRONIC MAIL 
00001160000ELECTRONIC MAIL AND NETWORKING
01703130101....... Call up another UNIX system
02704150101..... Send mail to users or read mail
03705150101..... Reports/changes permissions for users to "write" to you
04706160101.... Show name of current UNIX system
05707150101..... Copy files from one UNIX system to another UNIX system
06009120000MORE ELECTRONIC MAIL AND NETWORKING COMMANDS
07711160101.... Mail message processing system
08712160101.... Query a log of "uucp" transactions
09713170101... List the "uucp" names of known systems
10714170101... Accept or reject files transmitted via "uuto"
11715150101..... Public UNIX-to-UNIX system file copy
12716160101.... Write to another user on the same system

The Electronic Mail and Networking lists commands
that you can use to communicate with other users or
pass information via computer.

01"cu" is used to call up and log in to another terminal
or computer system.  In order to use "cu" to call up
another computer system, you must have a login name on
that system.  You can also use "cu" to transfer files
between your system and another UNIX system.
02"mail" prints your mail, message-by-message.  When
you login, you receive a message if you have mail.
By using the options, you can change the way your
mail is printed.  You may also send mail to people
on your system and other computer systems.  To do
this, you use the "persons" argument which names
the recipients of the mail.
03"mesg" can be used in two ways.  First, you can
use it to prevent or allow users to "write" to
your terminal.  Second, it can tell you whether
users are currently able to "write" to your
terminal.
04"uname" prints the name of the UNIX system you are
using.  Besides determining the system name, you
can retrieve other information such as the name
that the system is known by to a communication
network, system release and version, and machine
hardware name.
05"uucp" (UNIX system to UNIX system copy) copies
files from one UNIX system to another.  You may
send files from the current system to a remote
system, from a remote system to this system,
between two remote systems, or from one location
to another on this system.
07"mailx" is a command for sending and reading your
mail messages.  If you are sending mail, then
"mailx" lets you edit, review, and read other
files into the message.  If you are reading mail,
then "mailx" lets you save, delete, and respond
to the mail messages.
08"uulog" queries a summary log of "uucp" transactions
in the file /usr/spool/uucp/LOG/uucico/system.
09"uuname" lists the "uucp" names of systems known
to your system.  If a system name is printed by
"uuname", you can send "mail" to users on that
system, use "cu" to call up that system, and
"uucp" to copy files to the system.
10"uupick" allows you to accept or reject the files
transmitted to you via "uuto".  Specifically,
"uupick" searches a public directory, PUBDIR, for
files destined for you.
11"uuto" sends files to another UNIX System.  "uuto"
uses uucp to send the files. "uuto" allows the
receiving system to control the file access.
12"write" copies lines that you enter at your terminal
keyboard onto your terminal screen and to the
terminal screen of another user on your system.

010310cu
020410mail
030510mesg
040610uname
050710uucp
071110mailx
081210uulog
091310uuname
101410uupick
111510uuto
121610write

01cu
02mail
03mesg
04uname
05uucp
07
08
09
10
11
12


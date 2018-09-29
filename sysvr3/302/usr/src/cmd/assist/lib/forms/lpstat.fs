#ident	"@(#)forms.files:lpstat.fs	1.7"
00000lpstat
00901020000STATUS INFORMATION
01702060001PRINT SUMMARY STATUS INFORMATION [-s] (y/n):
02703060001PRINT ALL STATUS INFORMATION [-t] (y/n):
03704060050PRINT INFORMATION FOR REQUEST ID(S):
04705060001PRINT ACCEPTANCE STATUS FOR PRINTER(S) OR CLASS(ES) [-a]:
05706060001PRINT IDLE/DISABLED/ENABLED STATE FOR PRINTER(S) [-p]:
06007060030PRINT OUTPUT STATUS FOR PRINTER(S), CLASS(ES),
07708080001OR REQUEST ID(S) [-o]:
08709060001PRINT OUTPUT STATUS FOR USER(S) [-u]:
09710060001PRINT SCHEDULER STATUS [-r] (y/n):
10912020000CONFIGURATION INFORMATION
11713060001PRINT DEFAULT PRINTER DESTINATION [-d] (y/n):
12714060001PRINT MEMBERS OF CLASS(ES) [-c]:
13715060001PRINT DEVICE PATHNAMES OF PRINTER(S) [-v]:
14717060030ADDITIONS TO THE COMMAND LINE:

01082s
02092t
03062-
04020-
05030-
07040-
08050-
09072r
11132d
12110-
13120-
14142-


"lpstat" prints information about the current status
and configuration of the line printers on your system.
You may request multiple pieces of information at one time.

01Enter a "y" here if you want summary status information, including
the scheduler status, system default printer, a list of class names
with their members, and a list of printers with their devices. 
This is a good way to get lots of information simply.  [-s]
02Enter a "y" here if you want all status information. This
includes the status of the scheduler, the default printer, the 
device names of all printers, and the status of each printer.
This is a good way to get lots of information simply.  [-t]
03Enter a list of request ids on which you want status information.
The "lp" command creates a request id for each print request, and
prints out the request id on your terminal when the command is 
accepted.  The format of a request id is:  dest-seq
where "dest" is the destination printer, and "seq" is a sequence
number.  For example, "laser-1234" might be a request id on a
system with a printer called "laser."
This field is validated by ASSIST for format only.
ASSIST will not determine whether the request id actually exists.
04Enter a list of printer names and/or classes, to determine if
the printers are accepting jobs.  If you enter more than one
printer name or class, separate each by a comma, or place the
entire string in quotes, and separate each by a space and/or
comma.  Enter the value "all" (without quotes) to get this 
information for all printers. Printer names on your system are:
if [ -d /usr/spool/lp/member ];then ls -x /usr/spool/lp/member;else echo "not defined";fi
Printer classes on your system are:
if [ -d /usr/spool/lp/class ];then ls -x /usr/spool/lp/class;else echo "not defined";fi
[-a]
05Enter a list of printer names, to determine if the printers are 
idle, disabled, or enabled.  If you enter more than one printer,
separate each by a comma, or place the entire string in quotes
and separate each by a command and/or space.  Enter the value 
"all" (without quotes) to get this information for all printers. 
Printer names on your system are:
if [ -d /usr/spool/lp/member ];then ls -x /usr/spool/lp/member;else echo "not defined";fi
[-p]
07Enter a list of printer names, printer classes, and/or request ids, 
to determine the status of the named entities.  If you enter more
than one value, separate each by commas, or place the entire string
in quotes and separate each item by a comma and/or space.  Enter the
value "all" (without quotes) to get this information for all entities.
Printer names on your system are:
if [ -d /usr/spool/lp/member ];then ls -x /usr/spool/lp/member;else echo "not defined";fi
Printer classes on your system are:
if [ -d /usr/spool/lp/class ];then ls -x /usr/spool/lp/class;else echo "not defined";fi
[-p]
08Enter a list of login names, to determine the status of jobs
submitted by the named users.  If you enter more than one login,
separate each by a comma, or place quotes around the entire
string and separate each item by a command and/or space.
Enter the value "all" (without quotes) to get this information 
for all entities. [-u]
09Enter a "y" here if you want to know whether the 
scheduler is running.  The scheduler is used to coordinate the
submission of jobs to the appropriate printer.  If it is not
running, jobs will not be printed.  [-r]
11Enter a "y" here to find out what your system default destination is.
This is the printer that jobs will be sent to if you do not specify a
destination when you use the "lp" command (and the environment variable
LPDEST is not set). [-d]
12Enter a list of printer classes to determine the printers
belonging to each.  If you enter more than one class,
separate each by commas, or place the entire string in
quotes and separate each class by a space and/or comma.
Enter the value "all" (without quotes) to get this information 
for all known classes. Legal classes are:
if [ -d /usr/spool/lp/class ];then ls -x /usr/spool/lp/class;else echo "not defined";fi
[-c]
13Enter a list of printer names here.  For each you will be
given the device name (e.g. /dev/...) for that printer.  Legal
values are:
if [ -d /usr/spool/lp/member ];then ls -x /usr/spool/lp/member;else echo "not defined";fi
Enter the value "all" (without quotes) to get this information for
all printers.   You can use this information to find out the
physical device (e.g. /dev/xyz) associated with a particular printer
(e.g. laser).  [-v]
14ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0102
020109040507
0402
0502
0702
0902


010251n
020347n
091041n
111352n

0104yYnN
0105
0204yYnN
0205
0332/bin/sh $ASSISTLIB/forms/shell.lpstat r "$F03"
0432/bin/sh $ASSISTLIB/forms/shell.lpstat pc "$F04"
0405
0532/bin/sh $ASSISTLIB/forms/shell.lpstat p "$F05"
0505
0732/bin/sh $ASSISTLIB/forms/shell.lpstat pcu "$F07"
0705
0832/bin/sh $ASSISTLIB/forms/shell.lpstat u "$F08"
0904yYnN
0905
1104yYnN
1232/bin/sh $ASSISTLIB/forms/shell.lpstat c "$F12"
1332/bin/sh $ASSISTLIB/forms/shell.lpstat p "$F13"


011011y-sY-snN
021011y-tY-tnN
031110
041010-aall-a
051010-pall-p
061110
071010-oall-o
081010-uall-u
091011y-rY-rnN
111011y-dY-dnN
121010-call-c
131010-vall-v
141110




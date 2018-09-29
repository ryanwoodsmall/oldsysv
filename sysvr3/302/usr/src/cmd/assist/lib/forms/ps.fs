00000ps
00902020030TYPE OF PROCESSES
01703060001REPORT ON EVERY PROCESS RUNNING ON THIS SYSTEM [-e] (y/n):
02704060001REPORT ON ALL CHILD PROCESSES ON THIS SYSTEM [-d] (y/n):
03905060030REPORT ON CHILD PROCESSES THAT ARE
04706080001ASSOCIATED WITH A TERMINAL [-a] (y/n):
05707060030PROCESS IDS FOR PROCESSES YOU WANT REPORTS ON [-p]:
06708060030NUMBERS FOR TERMINALS THAT YOU WANT PROCESS DATA FOR [-t]:
07909060030PROCESS IDS FOR PROCESS GROUPS
08710080030WHOSE CHILD PROCESSES YOU WANT REPORTS ON [-g]:
09911060030USER IDS OR LOGIN NAMES OF
10712080030USERS WHOSE PROCESSES YOU WANT REPORTS ON [-u]:
11713060001NAME A SYSTEM TO REPORT PROCESS INFORMATION FROM [-n]:
12915020030TYPE OF INFORMATION
13716060001GENERATE A FULL LISTING OF INFORMATION [-f] (y/n):
14717060001GENERATE A LONG LISTING OF INFORMATION [-l] (y/n):
15719060001REDIRECT OUTPUT:
16720060050ADDITIONS TO THE COMMAND LINE:

01001
02001
04001
05060
06070
08080
10090
11100
13001
14001
15110
16122



"ps" reports the status of processes that are running on your system.
Without any options, "ps" prints information on processes associated
with your terminal.  By default, "ps" prints the process number, your
terminal line number, execution time, and the name of the active process
associated with your terminal.  With options, "ps" can report on processes
other than those running on your terminal and it can give more information
on each process.

01With this option "ps" prints information about every process 
that is running on your system. This option is incompatible
with "p", "t", "g", "u", "d", and "a". [-e]
02With this option "ps" prints information about all child processes
on this system.  This option does not print information about processes 
that start the child processes.  This option is incompatible with
"t", "p", "g", "u", "e", and "a". [-d]
04Select this option and "ps" prints information about child processes
associated with terminals on this system. It does not report on processes
that initiated the child processes nor does it report on processes that
were started by the system. This option is incompatible with "e", "d", 
"t", "p", "g", and "u". [-a]
05Type one or more process ID numbers for processes that you want 
information on.  Separate the process ID numbers from one another
using commas or spaces. If you use commas between the process ID numbers
you will save space on the command line because they will print like 
this:  -p 1005,239,4567.  Do not include a comma after the final number.
If you want to leave spaces between the process ID numbers, 
the command line will look like this: -p 1005  -p 239 -p 4567.
You can not use this option with "e", "d", or "a". [-p]
06Type terminal ID numbers for terminals whose processes you are 
interested in.  You can get a list of terminals that are currently
active on your system using the "who" command.  Separate the terminal ID 
numbers from one another using commas or blanks. If you use commas 
between the terminal ID numbers, you will save space on the command line 
because they will print like this: -t tty39,xt123,tty54.  Do not include 
a comma after the final number.  If you want to leave spaces between the
terminal ID numbers, the command line will look like: 
            -t tty39 -t xt123 -t tty54.  
If the terminal ID number begins with tty you only need to include the 
numeric part of the ID, e.g., tty39 in the above examples can be 
shortened to 39.  You can not use this option with  "e", "d", or "a". 
[-t]
08Type one or more process ID numbers to find out what child processes
were started from the processes.  When a process has child processes
it is a process group leader. For example, the "sh" command is often 
a process group leader because users type "sh" to start a new shell 
and then they execute commands in the new shell. When you type process 
ID numbers separate them from one another using commas or spaces. If 
you use commas between the numbers you will save space on the command 
line because they will print like this:  -g 1005,239,4567.  Do not 
include a comma after the final number.  If you want to leave spaces 
between the group leader process ID numbers the command line will look 
like this: -g 1005  -g 239 -g 4567.  You cannot use this option with 
"e", "d", or "a". [-g]
10Type one or more login names or user ID numbers for users whose 
processes you are interested in.  You can get a list of users who 
are currently working on your system by executing the "who" command.
If you want to use user ID numbers, they are listed in the /etc/passwd
file. Separate the login names or user ID numbers from one another using
commas or blanks. If you use commas between them, you will save space on
the command line because they will print like this: -u jane,jdoe,sls.
Do not include a comma after the final number.  If you want to leave 
spaces between the logins the command line will look like: 
              -u jane -t jdoe -t sls.  
You can not use this option with  "e", "d", or "a". [-u]
11Type the name of the system that "ps" should report on. By default,
"ps" reports on the system in the /unix directory.   The system that 
you select, must be the system that is running on your machine and it
must reside in the root directory. [-n]
13Select this option and in addition to the usual information,
"ps" prints the user ID of the user who initialized each process,
the process ID of the parent process, the process utilization
schedule, and the starting time of each process.  This option
is not compatible with "l" option. [-f]
14Select this option and in addition to the information "ps"
usually prints , it will print the flags that are associated
with the process, the state of the process, the user ID of the
user who initiated the process, the process ID of the parent
process, the process utilization schedule, the priority of the
process, the nice value of the process, the memory address of the
process, the size of memory image of the process in blocks, and
any event for which the process is waiting. This option is not
compatible with the "f" option. [-l]
15If you type a filename here, the output of this command will go to the 
file.  ASSIST will warn you if the file you type already exists.  If it 
exists, redirecting output to it will destroy the previous contents of 
the file.  (See the UNIX system walkthru for a more complete discussion 
of redirection.) 
16ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field,
you should use it with caution.

01020504060810
02010405060810
04010205060810
05010204
06010204
08010204
10010204
1314
1413


010365n
020463n
040647n
131657n
141757n

0104yYnN
0105
0204yYnN
0205
0404yYnN
0405
0505
0508,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
0605
0608,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
0632 if [ "`echo \"$F06\" | grep '[-!@#$%^&*()=+;:\./]'`" ]; then echo "COMMAS AND SPACES ARE THE ONLY LEGAL SEPARATOR CHARACTERS"; exit; fi; for i in `echo "$F06" | tr , " " | sed 's/  */ /g' | tr " " "\012"`; do if [ "`ps -t$i | wc -l`" -le 1 ]; then echo "THIS TERMINAL IS NOT IN USE: $i"; exit; fi; done
0805
0808,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
1005
1008,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
1032if [ "`echo \"$F10\"|grep '[-!@#$%^&*()=+;:\./]'`" ];then echo "COMMAS, SPACES ONLY LEGAL SEPARATOR CHARACTERS";exit;fi;for i in `echo "$F10"|tr , " "|sed 's/  */ /g'|tr " " "\012"`;do num=`cut -f3 -d: /etc/passwd|grep "^$i$"`;if [ -z "$num" ];then name=`grep "^$i:" /etc/passwd|cut -f1 -d":"`;else name=`grep ":$num:" /etc/passwd|/usr/bin/cut -f1 -d":"`;fi;if [ -z "$name" ];then echo "$i : NOT A VALID USER ID";exit;elif [ ! "`who|grep \"^$name \"`" ];then echo "$i NOT USING SYSTEM";exit;fi;done
113420001010
1100/
1304yYnN
1305
1404yYnN
1405
1503
1532if test -d $F15;then echo "$F15 IS A DIRECTORY";fi


001110
010001y-eY-enN
020001y-dY-dnN
031110
040001y-aY-anN
051110-p 
061110-t 
071110
081110-g 
091110
101110-u 
111010-n 
121110
130001y-fY-fnN
140001y-lY-lnN
151000> 
161110




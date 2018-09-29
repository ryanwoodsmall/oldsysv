00000who
00901020000USER INFORMATION
01702060001LONG LISTING OF INFORMATION ABOUT USERS [-u] (y/n):
02703060001STATE IF USER'S TERMINALS CAN BE WRITTEN TO [-T] (y/n):
03704060001PRINT COLUMN HEADINGS [-H] (y/n):
04705060001NAMES AND THE NUMBER OF USERS ON SYSTEM [-q] (y/n):
05706060001NAME OF FILE THAT CONTAINS USER INFORMATION [-a]:
06707060001PRINT YOUR OWN LOGIN NAME (y/n):
07908020000SYSTEM INFORMATION
08709060001LIST AVAILABLE TERMINAL LINES [-l] (y/n):
09710060001REPORT TIME AND DATE OF LAST SYSTEM START-UP [-b] (y/n):
10711060001REPORT LAST CHANGE OF SYSTEM CLOCK [-t] (y/n):
11712060001REPORT CURRENT RUN LEVEL OF OPERATING SYSTEM [-r] (y/n):
12713060001LIST PROCESSES THAT HAVE DIED AND NOT RESTARTED [-d] (y/n):
13714060001LIST ACTIVE SYSTEM INITIATED PROCESSES [-p] (y/n):
14716060001REDIRECT OUTPUT TO:
15717060030ADDITIONS TO THE COMMAND LINE: 

01020u
02030T
03040H
04050q
05060-a 
06072
08080l
09090b
10100t
11110r
12120d
13130p
14170>
15182


"who" gives you information about your system and the users on 
your system.  By default, "who" prints the names of users who 
are currently logged in to your system, their terminal numbers 
and the time that they logged in.  You can change user information 
you get by selecting an option from "USER INFORMATION", or get 
information on the system by selecting an option from "SYSTEM INFORMATION".

01"who" with this option gives you the user name, the line the user is
on, the time when the user logged in, the length of time since the
user last did some activity on the line, and a comment.
You can not use this option with -q or -s. [-u]
02With this option "who" prints user name, line number, and 
time that the user logged on, and it also prints a + when
other system users can write to the users terminal or a - when
other users can not. You can not use -b, -d, -r, -p, -q or -t 
with this option. [-T]
03Select this option to place column heading at 
the top of the columns of output for the "USER INFORMATION" options.  
You can only use this option with the options under "USER INFORMATION".
You can not use this option with -q.  [-H]
04Select this option, and "who" lists the users who are on the system 
and how many there are.  You can not use this option with any other 
options.[-q]
05Type the name of a file that "who" should look at for user information.
By default, "who" uses /etc/utmp.  The alternate file that is most often 
used is /etc/wtmp because it carries a history of all logins since it 
was created. This file has a special format and it is usually created by
the system not by a user.  "who" uses the information in the alternate 
file and gives information for all options except for -q. [-a]
06With this argument "who" tells your login name, line number and when
you logged on.  You can not use this with any options or arguments. 
Use this option to ask "who am i?"
08Select this option and "who" will print the available terminal
lines. You can not use this option with -t or -q. [-l]
09Select this option to get the time of the last system start up.
You can not use this option with -H, -T, -p, -q, -t, or -s. [-b]
10"who" with this option prints the last change to the system clock [-t].
11Reports a 1 if your operating system is running in single user mode.
Reports a 2 if your operating system is running in multi-user mode.
You can not use this option with -T, -H, -p, -t, -q, or -s. [-r]
12With this option "who" will print the names of processes that 
died and the time that they died. You can not
use this option with -T, -H, -p, -q  and -s. [-d]
13"who" with this option prints the names of the processes that the
system itself started.  You can not use this option with -T, -H, 
-b, -d, -r, or -q. [-p]
14If you type a filename here, the output of this command will go to the 
file.  ASSIST will warn you if the file you type already exists.  If it 
exists, redirecting output to it will destroy the previous contents of 
the file.  (See the UNIX system walkthru for a more complete discussion 
of redirection.) 
15ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

010604
0209110604101213
0309101213110604
040102030506080910111213
050604
0601020304050910111213
08060410
09030213100604
1003091106040208
11030213100604
120203130604
1303021209110604


010258n
020362n
030440n
040558n
060739n
080948n
091063n
101153n
111263n
121366n
131457n

0117yYnN
0105
0104yYnN
0205
0204yYnN
0305
0304yYnN
0405
0404yYnN
0514
0505
0502
0617yYnN
0604yYNn
0605
0805
0804yYnN
0905
0904yYnN
1005
1004yYnN
1105
1104yYnN
1205
1204yYnN
1305
1304yYnN
1403
1413
1432if test -d $F14;then echo "$F14 IS A DIRECTORY";fi


011011y-uY-unN
021011y-TY-TnN
031011y-HY-HnN
041011y-qY-qnN
051010-a 
060001y am iY am inN
081011y-lY-lnN
091011y-bY-bnN
101011y-tY-tnN
111011y-rY-rnN
121011y-dY-dnN
131011y-pY-pnN
141000> 
151110




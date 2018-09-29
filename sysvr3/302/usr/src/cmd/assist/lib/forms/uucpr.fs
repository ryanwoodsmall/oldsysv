#ident	"@(#)forms.files:uucpr.fs	1.7"
00000uucp
00701060101REMOTE SYSTEM NAME(S):
01702060101REMOTE SOURCE FILE NAME:
02703060101LOCAL DESTINATION FILE OR DIRECTORY NAME:
03904020030OPTIONAL UUCP FEATURES
04705060001WHEN FILES ARE COPIED SEND MAIL TO SENDER [-m] (y/n):
05706060001FILE NAME WHERE TRANSFER STATUS REPORT SHOULD BE PLACED [-s]:
06707060001COPY SOURCE FILES TO SPOOL DIRECTORY FOR TRANSFER [-C] (y/n):
07708060001MAKE INTERMEDIATE DIRECTORIES FOR THE FILE COPY [-f] (y/n):
08709060001PRINT JOB IDENTIFIER ON TERMINAL [-j] (y/n):
09710060001PRIORITY OF JOB [-g]:
10711060001QUEUE JOB ONLY, DO NOT START FILE TRANSFER [-r] (y/n):
11712060001DEBUGGING LEVEL [-x]:
12713060030ADDITIONS TO THE COMMAND LINE:

00040
01052
02060
04032
05002
06012
07022
08002
09000
10002
11002
12072


"uucp" (UNIX system to UNIX system copy) copies
files from one UNIX system to another.  You may
send files from the current system to a remote
system or from a remote system to this system.
This command form lets you use "uucp" to copy files 
to this system (i. e., /bin/uname -n) from a remote system.  
The other "uucp" command form allows you to send a file
from this system to a remote system.

00Enter the uucp name of the system where your file is,
followed by a "!".  The remote system name must be "known" to this 
system.  To see the uucp node names of systems "known" to this system, 
use the "uuname" command.  There may be times that you want to get
a file from a system not known to your system.  In this case, you may
specify a list of intermediate system names, each followed by a "!".
Your system administrator or the administrator of the remote system 
may be able to give you this list of intermediate systems.  In any
case, the first system name in the list must be known to your system.
01Enter the pathname of the files to be copied here.  These files must 
exist on this system and must either have general read permissions 
or be owned and readable by you.  The only way to specify multiple
files is to use shell metacharacters (e.g. *).  These characters will 
be expanded on the remote system.  This field is not validated by ASSIST.
02Enter the pathname of a file or directory on this system, where
the file should be placed.  (If you are copying more than one 
source file, you must specify a directory.)  The destination path 
must have general read and write permissions.  There may be 
additional restrictions on what directories you can copy files to 
(and this will vary from system to system).  To be on the safe side,
send your files to a path under /usr/spool/uucppublic.  You can do
this by entering ~/ followed by a pathname.  (For example, ~/echo $LOGNAME/
will send files to the directory /usr/spool/uucppublic/echo $LOGNAME/
on this system.)
04Enter a "y" here if you want to receive mail when the transfer
of files is complete. [-m]
05Enter here the full pathname of the file on this system
where a status report of the transfer should be placed. [-s]
06By default, the source file you specify will be used
to make the "uucp" copy.  Since the uucp copy is not always
made right away,  you may want to have a copy of the source
file saved on the "spool" directory (this is a public directory
used to save files pending the completion of some job) for
use in making the final "uucp" copy.  Type a "y" here if you 
want this option. [-C]
07By default, "uucp" will create directories in the destination 
pathname that do not already exist on this system.
If you type "n" here, "uucp" will not create any directories 
and the copy will succeed only if all directories in the 
destination pathname already exist. [-f]
08"uucp" creates a job identification string for each request
that it accepts.  This job identifier can be used with the
"uustat" command to obtain the status of your job, or
to terminate the job.  By default, the job identifier is
not visible to you.  Select this option to have "uucp"
print the job id on your terminal.  [-j]
09You may enter a single letter or digit in this field
to specify the priority of a job.  The higher the
priority of a job, the sooner the transfer will take
place within a conversation.  In other words, if several
jobs are queued to be copied to a particular machine,
the jobs with the highest priority will be copied
before jobs with lower priority.  The smaller the
digit you enter here, the higher will be the job's
priority.  Likewise, the lower the letter, the higher
the priority (e.g. "a" before "z").  [-g]
10Select this option if you want your job placed on
the "uucp" queue, but you do not want the file
transfer to take place at this time.  [-r]
11You may use this option to request debugging information.
Legal values are the digits 0 through 9.  The higher
the number, the more detailed information you will
receive.  [-x]
12ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



040560n
060768n
070866y
080951n
101161n

0032tmp="`echo $F00 | /usr/bin/cut -f1 -d!`";if [ ! "`/usr/bin/uuname | /bin/grep $tmp`" ];then echo "$tmp IS NOT A SYSTEM NAME KNOWN TO THIS SYSTEM";fi
0017!
0404yYnN
053420101010
0503
0517/
0604yYnN
0704nNyY
0804yYnN
0908
1004yYnN
1108


001000
010010
020010
031110
041011y-mY-mnN
051010-s
061011y-CY-CnN
071011n-fN-fyY
081011y-jY-jnN
091010-g
101011y-rY-rnN
111010-x
121110




#ident	"@(#)forms.files:uucpl.fs	1.5"
00000uucp
00701060130LOCAL SOURCE FILE NAME(S):
01702060101REMOTE SYSTEM NAME(S):
02703060101REMOTE DESTINATION FILE OR DIRECTORY NAME:
03905020030OPTIONAL UUCP FEATURES
04706060001REMOTE USER TO WHOM ARRIVAL MAIL SHOULD BE SENT [-n]:
05707060001WHEN FILES ARE COPIED SEND MAIL TO SENDER [-m] (y/n):
06708060001FILE NAME WHERE TRANSFER STATUS REPORT SHOULD BE PLACED [-s]:
07709060001COPY SOURCE FILES TO SPOOL DIRECTORY FOR TRANSFER [-C] (y/n):
08710060001MAKE INTERMEDIATE DIRECTORIES FOR THE FILE COPY [-f] (y/n):
09711060001PRINT JOB IDENTIFIER ON TERMINAL [-j] (y/n):
10712060001PRIORITY OF JOB [-g]:
11713060001QUEUE JOB ONLY, DO NOT START FILE TRANSFER [-r] (y/n):
12714060001DEBUGGING LEVEL [-x]:
13715060030ADDITIONS TO THE COMMAND LINE:

00050
01060
02072
04010
05022
06000
07032
08042
09002
10000
11002
12002
13082


"uucp" (UNIX system to UNIX system copy) copies
files from one UNIX system to another.  You may
send files from the current system to a remote
system or from a remote system to this system.
This command form lets you use "uucp" to send files 
from this system (i. e., /bin/uname -n) to a remote system.  
The other "uucp" command form allows you to send files
from a remote system to this one.

00Enter the pathname of one or more files here.  These files must 
exist on this system and must either have general read permissions 
or be owned and readable by you.  If you specify more than one file, 
you must enter the name of a directory as part of the destination 
address below.
01Enter the uucp name of the system where you want the file sent,
followed by a "!".  The remote system name must be "known" to this 
system.  To see the uucp node names of systems "known" to this system, 
use the "uuname" command.  There may be times that you want to send
a file to a system not known to your system.  In this case, you may
specify a list of intermediate system names, each followed by a "!".
Your system administrator or the administrator of the remote system 
may be able to give you this list of intermediate systems.  In any
case, the first system name in the list must be known to your system.
02Enter the pathname of a file or directory on the destination 
system.  (If you are sending more than one source file, you must 
specify a directory.)  The destination path must have general 
read and write permissions on the remote system.  There may be 
additional restrictions on what directories you can send files to 
(and this will vary from system to system).  To be on the safe side,
send your files to a path under /usr/spool/uucppublic.  You can do
this by entering ~/ followed by a pathname.  (For example, ~/echo $LOGNAME/
will send files to the directory /usr/spool/uucppublic/echo $LOGNAME/
on the remote system.)
04Enter the user login name of the person on the remote system
to whom mail should be sent.  This mail will notify the user
that the files have arrived, and where they can be found.  
This field is not validated by ASSIST. [-n]
05Enter a "y" here if you want to receive mail when the transfer
of files is complete. [-m]
06Enter here the full pathname of the file on this system
where a status report of the transfer should be placed. [-s]
07By default, the source file you specify will be used
to make the "uucp" copy.  Since the uucp copy is not always
made right away,  you may want to have a copy of the source
file saved on the "spool" directory (this is a public directory
used to save files pending the completion of some job) for
use in making the final "uucp" copy.  Type a "y" here if you 
want this option. [-C]
08By default, "uucp" will create directories in the destination 
pathname that do not already exist on the destination system.
If you type "n" here, "uucp" will not create any directories 
and the copy will succeed only if all directories in the 
destination pathname already exist. [-f]
09"uucp" creates a job identification string for each request
that it accepts.  This job identifier can be used with the
"uustat" command to obtain the status of your job, or
to terminate the job.  By default, the job identifier is
not visible to you.  Select this option to have "uucp"
print the job id on your terminal.  [-j]
10You may enter a single letter or digit in this field
to specify the priority of a job.  The higher the
priority of a job, the sooner the transfer will take
place within a conversation.  In other words, if several
jobs are queued to be copied to a particular machine,
the jobs with the highest priority will be copied
before jobs with lower priority.  The smaller the
digit you enter here, the higher will be the job's
priority.  Likewise, the lower the letter, the higher
the priority (e.g. "a" before "z").  [-g]
11Select this option if you want your job placed on
the "uucp" queue, but you do not want the file
transfer to take place at this time. [-r]
12You may use this option to request debugging information.
Legal values are the digits 0 through 9.  The higher
the number, the more detailed information you will
receive.  [-x]
13ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



050760n
070968n
081066y
091151n
111361n

003420001010
0002
0132tmp="`echo $F01 | /usr/bin/cut -f1 -d!`";if [ ! "`/usr/bin/uuname | /bin/grep $tmp`" ];then echo "$tmp IS NOT A SYSTEM NAME KNOWN TO THIS SYSTEM";fi
0131
0117!
0504yYnN
0603
0617/
0704yYnN
0804nNyY
0904yYnN
1008
1104yYnN
1208


001110
011000
020010
031110
041010-n
051011y-mY-mnN
061010-s
071011y-CY-CnN
081011n-fN-fyY
091011y-jY-jnN
101010-g
111011y-rY-rnN
121010-x
131110

0104



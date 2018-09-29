#ident	"@(#)forms.files:lp.fs	1.3"
00000lp
00701060030FILE(S) TO BE PRINTED:
01702060001SEND MAIL WHEN FILES ARE PRINTED [-m] (y/n):
02703060030PRINTER-DEPENDENT OPTIONS [-o]:
03704060001TITLE ON BANNER PAGE [-t]:
04705060001NUMBER OF COPIES TO PRINT [-n]:
05706060001DESTINATION PRINTER [-d]:
06707060001WRITE MESSAGE TO SCREEN WHEN FILES ARE PRINTED [-w] (y/n):
07708060001COPY FILES IMMEDIATELY [-c] (y/n):
08709060001SUPPRESS INFORMATION MESSAGES TO SCREEN [-s] (y/n):
09710060030ADDITIONS TO THE COMMAND LINE:

00090-
01030m
02050-o
03070-t
04040-n1
05020-dif [ "$LPDEST" ];then echo $LPDEST;elif [ -r /usr/spool/lp/default ];then cat /usr/spool/lp/default;fi
06080w
07010c
08060s
09102-

Since you did not enter an input file (or hyphen), 
"lp" will expect input to come from your terminal.  Enter ^D 
to indicate to "lp" that you have completed your input.00-

"lp" sends files to a line printer.  If you do not
include any file names, "lp" will take input from
standard input.

00Enter the names of files to be printed.  If you leave
this field blank, or include a hyphen on the command line,
input will be taken from standard input.  In this case, you
must type control-d (^D) to terminate input.
01If you want to receive mail when your files 
have been printed, select this option. [-m]
02This field is used to specify printer-dependent options.  For example,
"mode=s" specifies single sided output for certain printers.  You may
enter multiple options.  See your system administrator for the
printer-dependent options available on your system.
ASSIST will not perform any error checking on this field [-o].
03If you want a specific message to appear on the banner page of your
output, enter a string here.  Use quotes for messages that contain
spaces. For example, the string "Progress Report" will cause
this message to appear on the first page of your printout.  This
makes it easy for you to identify the particular job.  [-t]
04By default, one copy of each named file is printed.  If you 
want more than one copy, enter the desired number of copies here.
05Use this field to specify a destination printer other than the default.
If the environment variable LPDEST is set, the destination will default
to this.  If LPDEST is not set, and you do not specify a value here,
the destination will be your system default.
      lpstat -d
Legal values are:
if [ -d /usr/spool/lp/member ]; then ls -x /usr/spool/lp/member;fi
Your entry must come from this list.  [-d]
06If you want to be informed directly on your screen when your files
have been printed, select this option.  With this option, the "lp"
message will appear directly on your screen in the middle of
whatever you are doing. If you are not logged in when the files 
are printed, mail will be sent instead. [-w]
07There is usually a delay between the time that you ask that a file
be printed, and the time when the file is actually printed.  Normally,
if you make any changes to the file in this interval, those changes
will appear in the printed output.  If you want to force the command
to print the file as it exists now, select this option. [-c]
08The command prints some information messages on your screen.
If you want to suppress such messages, select this option. [-s]
09ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0608


010251n
0405381
050632if [ "$LPDEST" ];then echo $LPDEST;elif [ -r /usr/spool/lp/default ];then cat /usr/spool/lp/default;fi
060765n
070841n
080958n

0001
0015-
0013
0104yYnN
0408
0432if [ $F04 -eq 0 ];then echo "MUST BE GREATER THAN ZERO";fi
0532if [ ! "`ls /usr/spool/lp/member|grep $F05`" ];then echo "UNKNOWN DESTINATION.  SEE ITEM HELP FOR MORE INFORMATION";fi
0605
0604yYnN
0704yYnN
0804yYnN


001110
011011y-mY-mnN
021110-o
031010-t
041010-n1
051010-dif [ "$LPDEST" ];then echo $LPDEST;elif [ -r /usr/spool/lp/default ];then cat /usr/spool/lp/default;fi
061011y-wY-wnN
071011y-cY-cnN
081011y-sY-snN
091110




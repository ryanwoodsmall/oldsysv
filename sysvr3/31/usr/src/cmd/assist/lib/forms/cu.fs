#ident	"@(#)forms.files:cu.fs	1.7"
00000cu
00701060001TELEPHONE NUMBER:
01703060001UUCP SYSTEM NAME:
02705060001LINE [-l]:
03707060001TRANSMISSION SPEED [-s]:
04709060001SEND EVEN, ODD, OR NO PARITY [-e -o] (e/o/n):
05711060001PROMPT FOR TELEPHONE NUMBER [-n] (y/n):
06713060001TURN ON LOCAL ECHO [-h] (y/n):
07715060001CALLING AN AUTO-ANSWER TERMINAL [-t] (y/n):
08717060001PRINT DIAGNOSTICS [-d] (y/n):
09719060030ADDITIONS TO THE COMMAND LINE:

00070-
01070
02060-l
03050-s300
04040-n
05001
06001h
07001t
08001d
09092


"cu" is used to call up and log in to another terminal
or computer system.  In order to use "cu" to call up
another computer system, you must have a login name on
that system.  You can also use "cu" to transfer files
between your system and another UNIX system.

00If you are using an auto-dialer, enter the telephone number of
the remote system here.  Equal signs (=) will wait for secondary
dialtones, while hyphens (-) will insert delays.
You must either provide a telephone number, an UUCP system name,
or a device name.
01Enter here the uucp name of the system to which you
wish to connect.  You must either specify a system
name in this field, or provide a value for the telephone
or line fields (or select the "-n" option).
02This field is used to specify a device name to be used as the
communication line.  Usually, this is a directly connected
synchronous line, in which case no telephone number is needed.
If the device is an auto-dialer, you must include a telephone
number.  No validation is performed by ASSIST on this field. [-l]
03You can use this field to set the transmission speed.  Legal 
values are 300, 1200, 2400, 4800, or 9600. [-s]
04By default, no parity data are generated to send to the remote
system.  Enter an "e" if you want even parity sent, or an "o" if
you want odd parity sent.  [-e -o]
05Select this option if you would prefer to be prompted
for the telephone number. If you select this option,
the "cu" command will prompt you for a number when you
execute this command form. [-n]
06If you are communicating with a remote system that expects its
terminals to be in half-duplex mode, set this option to "y". [-h]
07This option should be set to "y" when you are dialing up an
auto answer terminal.  This maps a RETURN to a RETURN-NEW LINE
pair. [-t]
08Select this option if you want diagnostic 
messages to be printed. [-d]
09ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0001
010002050307
0201
0301
050001
070102


040952n
051146n
061337n
071550n
081736n

0005
0032if [ "`echo $F00 | /bin/sed -n '/[^0123456789=\-]/p'`" ];then echo "ONLY THE DIGITS 0-9, HYPHEN, AND EQUAL SIGN ALLOWED";fi
0031
0105
0132if [ ! "`/usr/bin/uuname | /bin/grep $F01`" ];then echo "NO SUCH UUCP SYSTEM NAME";fi
0131
0205
0231
0305
03043001200240048009600
0404eon
0531
0504yYnN
0505
0604yYnN
0704yYnN
0705
0804yYnN


001110
011110
021010-l 
031010-s 
041010-n
050001y-nY-nnN
060001y-hY-hnN
070001y-tY-tnN
080001y-dY-dnN
091110

00020105
01020005
02010005
05020100



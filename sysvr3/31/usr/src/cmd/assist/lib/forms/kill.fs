#ident	"@(#)forms.files:kill.fs	1.2"
00000kill
00701060130ID NUMBER(S) OF PROCESS(ES):
01703060101SEND SIGNAL NUMBER:
02705060030ADDITIONS TO THE COMMAND LINE:

00020
01012-15
02032


You use "kill" to terminate processes; more generally, you can use
"kill" to send to send any signal number to the specified processes.
The default signal number used by "kill" to terminate processes is 15.

00You identify the process you want to "kill" (or send a signal to) by its 
process id number.  You can use the ps(1) command to identify process 
ID numbers of your active processes.  You must own the process you want 
to "kill".  Use the number 0 to signal all processes.
01By default, "kill" sends the signal 15 to terminate a process.  You can
read a list of all possible signals under signal(2) in the Programmer's
Manual.  The signal 9 produces a "sure kill".
02ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



01032615

0008
0032if test "$F00" = "0";then exit;fi;ps -u $LOGNAME | grep "^ *$F00">/dev/null;if test $? -ne 0;then echo "$F00 not good process";fi
0108122123456789


001110
011010-15
021110




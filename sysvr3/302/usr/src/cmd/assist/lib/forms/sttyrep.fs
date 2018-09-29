#ident	"@(#)forms.files:sttyrep.fs	1.2"
00000stty
00704050101REPORT CURRENT SETTINGS OF SOME "stty" TERMINAL OPTIONS (y/n):
01706050001REPORT ALL CURRENT "stty" TERMINAL OPTION SETTINGS [-a] (y/n):
02008050030REPORT CURRENT SETTINGS AS ARGUMENT
03709070101FOR ANOTHER "stty" COMMAND [-g] (y/n):
04711050030ADDITIONS TO THE COMMAND LINE:

00020
01030
03040
04052


This is the form you will use in ASSIST to report your "stty" I/O
settings.  Use the first field in the form to view some of your
"stty" I/O settings or the second field, "-a", to view all of your
current settings.  Then go back to the previous menu using "^R" and
select the 'set "stty" options' field to make appropriate changes in
the "stty" I/O settings.  Caution should be used when running  "stty"
because the command does no sanity checking.  Some sanity checking
is provided by ASSIST.

00This field when invoked will report some current I/O terminal settings.
This is what happens when you run the command "stty" without any
arguments so after choosing this field,  you will see only "stty"
on the command line.
01This field, when invoked, will report all current 
I/O terminal settings.  The I/O settings will be
printed on your screen.   [-a]
03If you answer "y" in this field, then you
will get a report of current "stty" settings
in a form that can be used as an argument to
another "stty" command.  [-g]
04ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

000103
010003
030001


000468y
010668n
030946n

0004yYnN
0031
0005
0104yYnN
0131
0105
0304yYnN
0331
0305


001011y Y nN
011011y-aY-anN
021110
031011y-gY-gnN
041110

000301
010003
030001



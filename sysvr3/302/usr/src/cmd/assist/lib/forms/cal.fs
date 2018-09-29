#ident	"@(#)forms.files:cal.fs	1.3"
00000cal
00702060001YEAR (1-9999):
01704060001MONTH (1-12):
02706060030ADDITIONS TO THE COMMAND LINE:

00022
01012
02032


"cal" prints a calendar on your terminal.  If you
specify neither a month nor a year, "cal" will print
a calendar for the current month in the current year.

00By default, the "cal" command assumes the current year.  
Enter a value here if you want a different year.  
You must also specify a year if you plan to enter a month.  
If you enter a year but no month, you will get a calendar 
for the entire year.
01By default, the "cal" command assumes the current month.  
Enter a value here if you want a different month.  
You must use the numeric (1-12) representation of the month.  
In order to specify a value here, you must also supply a year.
02ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.


0100


0019
0008
0032if [ "$F00" -lt 1 -o "$F00" -gt 9999 ]; then echo "THE YEAR MUST BE A NUMBER BETWEEN 1 AND 9999";fi
0116year
0108
0132if [ "$F01" -lt 1 -o "$F01" -gt 12 ];then echo "MONTH MUST BE EXPRESSED AS A NUMBER BETWEEN 1 AND 12";fi


001110
011110
021110




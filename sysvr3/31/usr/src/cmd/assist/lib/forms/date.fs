00000date
00701060001CHANGE THE FORMAT OF DATE TO:
01703060001CHANGE THE DATE TO:
02705060030ADDITIONS TO THE COMMAND LINE:

00012
01022
02032


"date" prints the date and time.  You can change the format
of how the date and time are printed by specifying the format
in the CHANGE THE FORMAT OF DATE TO field.  You can also change
the date for your computer, but you must be super user to do so.

00You can combine the arguments listed below with any alphanumeric, 
special characters, or white space to change the format of "date". Begin 
the argument string with a plus(+), and surround the entire string with 
single quotes (''). Example:  If you type '+DAY: %a%nDATE: %d' on Sunday 
the 3rd, "date" prints    DAY: Sun
                          DATE: 03
%m  month of the year  %d  day of the month  %w  day of the week, 0=Sunday
%j  day of the year    %h  abbreviated month %a  abbreviated weekday
%M  minutes            %S  seconds           %r   AM/PM notation
  %D  date as month/date/year          %y  last two digits of the year 
  %H  hour in military time 00 to 23   %T  time in hours:minutes:seconds 
  %n  insert a new line                %t  insert a tab
01This field enables you to change the date on your computer.
You must be super-user to change the date.  You change the 
date by specifying the month, day of the month, hour, minute, 
and year in two digit numbers.  For example, 10080045 sets the 
date and time to 12:45 AM October 8.  The year is always set 
to the current year unless you specify a different year.
02ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0001
0100



0005
0017'+'
0032s=\'"$F00"\'; eval /bin/date $s >/dev/null 2> /dev/null; if test $? != 0;then echo "$F00 NOT VALID date FORMAT. SEE HELP MESSAGE FOR FORMAT RULES"; fi;
0105
0132if [ ! "`/usr/bin/id|/bin/grep 'uid=0'`" ]; then echo "YOU MUST BE SUPER-USER TO CHANGE THE DATE";fi
0108


001010
011010
021110




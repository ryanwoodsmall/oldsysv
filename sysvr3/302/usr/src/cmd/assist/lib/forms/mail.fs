00000mail
00901020000READ MAIL
01702060001NAME OF MAIL FILE TO USE INSTEAD OF DEFAULT [-f]:
02703060001PRINT MESSAGES IN FIRST-IN, FIRST-OUT ORDER  [-r] (y/n):
03004060001PRINT MAIL WITHOUT PROMPTING FOR DISPOSITION
04705080001OF MESSAGE [-p] (y/n):
05706060001TERMINATE "mail" AFTER AN INTERRUPT [-q] (y/n):
06707060001REPORT MAIL WAITING, BUT DO NOT PRINT IT [-e] (y/n):
07708060001DISPLAY WINDOW OF TOP-MOST HEADERS [-h] (y/n):
08709060001FORWARD MAIL TO [-F]:
09911020000SEND MAIL
10712060030LOGIN NAME OF PERSON(S) RECEIVING MESSAGE:
11713060001PRECEDE MESSAGE BY LOGIN NAMES OF RECIPIENTS [-t] (y/n):
12714060001SUPPRESS ADDRESS OPTIMIZATION [-o] (y/n):
13715060001SUPPRESS ADDITION OF NEW LINE AT TOP OF LETTER [-s] (y/n):
14016060030SEND LETTER WITHOUT WAITING FOR REMOTE
15717080001TRANSFER COMPLETION [-w] (y/n):
16719060030ADDITIONS TO THE COMMAND LINE: 

01020-fmailfile
02001r
04001
05001q
06001e
07001
08030
10020
11001
12001
13001
15001
16042

If you are reading mail, ignore this message.
If you are sending mail, when you execute the "mail" command you will be
put into an interactive mode.  Type your mail message.  When you have
finished typing the message, strike RETURN.  Then strike ^D or type "."
on a line by itself.  This indicates that your message is complete.  

"mail" without arguments prints your mail, message-by-message, in
last-in, first-out order.  For each message, you are prompted with a "?".
Your response is read from the standard input to determine the disposition
of the message.  You may make the following responses to the "?"
prompt:
<new-line>, +, n: go to next message
d                 delete message & go to next message
p                 print message again
-                 go back to previous message
s [files], y      save message in named files
w [files]         save message without topmost header in files
m [persons]       mail message to named persons
q, EOT            put undeleted mail back in mailfile and stop
x                 put all mail back in mailfile, unchanged & stop
!command          escape to the shell to do command
?                 print a command summary
"mail" also allows you to send mail.  You can send mail to somebody
on a remote system if you have the Basic Networking Utilities installed.

01This file is an existing file.  Use this option when you want to read
mail from a file other than the default "mbox."  For example, if you keep
a separate file for mail pertaining to a particular subject, you can 
review this special mailfile and perform the same actions on it as you 
would on the default "mailfile."  The file must begin with "FROM" followed
by the path the mail took to reach its destination.
This option is incompatible with -e and -t.  [-f]
02This option prints your mail messages in first-in first-out order.  The
default order is last-in first-out.  This option is incompatible with
-e and -t. [-r]
04This option prints your messages without asking you how you want
to dispose of them.  Normally, "mail" asks you how you want to
dispose of your messages after you've read them. [-p]
05This option terminates "mail" after interrupts such as DEL or
BREAK.  Normally, an interrupt only causes the termination of
the message being printed.  This option is incompatible with
-e and -t.  [-q]
06This option tells you if you have mail waiting to be read, but it does
not print the messages.  It is incompatible with -p, -q, -r, -f, and 
-t. [-e]
07Normally, "mail" prints the letters in your mailfile one
at a time.  This option only prints the headers, which tell
you who each piece of mail is from rather than the letters.
It is incompatible with -e, -p, -r, -o, -s, -w, -t. [-h]
08This option forwards your mail.  If you are in a multi-machine
environment, this option forwards all of your mail to a single
machine.  If you are forwarding mail to a single user, type the
system name and the user login, e.g., system_name!user_login.
If you are forwarding mail to more than one person, you must
enclose all of the names in double quotes so that everything
can be interpreted as the operand of the -F option.  To remove
the forwarding feature, type:  mail -F"". [-F]
10List the login name(s) of the person(s) you are sending mail.
If the person is not recognized by the system, your message 
will be saved in a temporary file called "dead.letter."  This
option is incompatible with -q, -r, and -f.  
11This option prints the names of all recipients at the top of the
mail message before the reader reads it.  The names are preceded 
by the word To:. [-t] 
12This option suppresses the address optimization
facility. [-o]
13Normally, when you send mail, "mail" puts a new line at the
top of the letter you are sending.  This option suppresses
the new line. [-s]
15When you send mail to a remote location, the letter is not
sent until the remote transfer has taken place.  If you
use this option, the letter is sent without waiting for
the remote transfer to complete. [-w]
16MUSE appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
MUSE does not perform any error checking on this field,
you should use it with caution.

01121315061011
0212131506071011
0412131506071011
05121315061011
060301020405121315071011
071513120604021011
081011121315
100301020405060708
1101020405060708
1201020405060708
1301020405060708
1501020405060708


020363n
040531n
050654n
060759n
070853n
111363n
121448n
131565n
151740n

0102
013420000010
0105
0204yYnN
0205
0404yYnN
0405
0504yYnN
0505
0604yYnN
0605
0704yYnN
0705
0735
0805
0832if [ "`echo $F08 | /bin/grep !`" = "$F08" ] ; then if /usr/bin/uuname | /bin/grep `/bin/expr $F08 : '\(.*\)!.*'`> /dev/null; then exit; else echo "$F08 DOES NOT CONTAIN A VALID SYSTEM NAME"; fi; else if [ "$F08" != "`/bin/grep $F08 /etc/passwd | /bin/sed \"s/\([^:]*\):.*/\1/\"`" ] ; then echo "$F08 IS NOT A VALID LOGIN NAME"; fi;fi;
0835
1019
1005
1032if [ "$F10" != "`/bin/grep $F10 /etc/passwd | /bin/sed \"s/\([^:]*\):.*/\1/\"`" ]; then echo "$F10 IS NOT A VALID LOGIN NAME"; fi;
1105
1104yYnN
1204yYnN
1205
1235
1304yYnN
1305
1335
1504yYnN
1505
1535


011010-f 
020001y-rY-rnN
030001y-pY-pnN
040001y-pY-pnN
050001y-qY-qnN
060001y-eY-enN
070001y-hY-hnN
081010-F
101110
110001y-tY-tnN
120001y-oY-onN
130001y-sY-snN
141110
150001y-wY-wnN
161110




00000passwd
00901060030"passwd" is an interactive command that will prompt you for input
01902080030after you execute the command.
02704060001LOGIN NAME: 

02010


"passwd" changes or installs a password for your login name.
"passwd" prompts you for your current password, if any, and
then prompts you twice for your new password.  Passwords must
have at least six characters.  These six characters must contain
at least two alphabetic characters and at least one numeric
or special character.  Passwords must differ from your login name.
Also, your new password must differ from your old by at least
three characters.

02"passwd" changes or installs the password associated with
the login name.  If you want to change your password,
enter your login name here.




0232ID=`id | sed -e " s/[^ 	]*(\([^ 	]*\)).*/\1/"`; if [ $ID != "root" ] ; then if [ "$F02" != $ID ] ; then echo "$ID CANNOT CHANGE PASSWORD FOR $F02"; exit; fi; fi; if [ "$F02" != "`grep $F02 /etc/passwd | sed \"s/\([^:]*\):.*/\1/\"`" ]; then echo "$F02 IS NOT A VALID LOGIN NAME"; fi; 


001110
011110
021110




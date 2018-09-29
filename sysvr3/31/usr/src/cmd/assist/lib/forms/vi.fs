#ident	"@(#)forms.files:vi.fs	1.5"
00000vi
00701060030FILE NAME(S):
01703060001DO THIS "ex" COMMAND AS THE FIRST COMMAND [-c]:
02705060101SET READ-ONLY MODE [-R] (y/n):
03707060101DEFAULT WINDOW SIZE [-w]:
04709060001EDIT THE FILE SPECIFIED BY THE TAG [-t]:
05912020000RECOVER WORK SAVED IN SYSTEM CRASH
06714060001EDIT FILE SAVED IN SYSTEM CRASH [-r]:
07716060101LIST ALL FILES SAVED IN SYSTEM CRASH [-L] (y/n):
08919020030ENCRYPTION/DECRYPTION OF FILES
09021060030READ IN FILE ASSUMING FILE IS ENCRYPTED AND
10722080101WRITE OUT FILE IN ENCRYPTED FORM [-C] (y/n):
11024060030READ IN FILE HAVING "vi" MAKE EDUCATED GUESS TO SEE IF
12725080101FILE IS ENCRYPTED AND WRITE OUT FILE IN ENCRYPTED FORM [-x] (y/n):

00090
01020+
02030-R
03040-wset `/bin/stty`;if test $2 -gt 1200;then /bin/echo 23;elif test $2 -gt 300;then /bin/echo 16;else /bin/echo 5;fi
04050-t 
06070-r 
07080-r
10060
12060-x

"vi" is an interactive command.
Use the command :q!<CR> to exit "vi" without saving your editing changes.
Use the command :wq<CR> to save your changes and exit "vi".
Strike the <ESCAPE> key if editing commands appear on the screen as text.

"vi" is a full-screen text editor.  When you use "vi", the screen shows
the editing changes you make as you make them.  You can use "vi" 
to create and edit any type of text file.  Examples of text files 
include (1) mail messages, (2) documents that will be formatted by a 
command such as "mm", (3) programs in languages such as C, and (4) shell 
scripts.  

00Enter the name(s) of files you want to edit here. 
01"vi" will execute the "ex" (:) command you put here before beginning
the editing session.  ASSIST does not check whether your expression is
a valid "ex" command. 

Use single quotes around the expression you enter. 

For example, suppose the file "mary" contains the rhyme "Mary Had a
Little Lamb." Entering '/everywhere/' in this "ex" field would
position the cursor on the third line ("and everywhere that ...") of
the file "mary" instead of the first line.  Entering the "ex" command
'g/Mary/s/Mary/Sally/' would edit the file to read "Sally had ..' [-c]
02If you enter "y" in this field, you will be able to read a file
in "vi", but you will not be able to write your editing changes back
to the file.  Use this option to prevent any accidental overwriting
of the file. [-R]
03The number of lines "vi" draws on your screen depends on the speed
of your terminal line.  For example, at 300 baud, "vi" draws 5 lines
by default.  The "vi" command form shows the default number of lines for
the baud rate you are currently using.  Use this option to change the
number of lines to some number other than the default. [-w]
04Use this option when you have several files
     -and want to specify which file you want to edit
     -and where you will position the cursor at the start of editing
The tags file is a file in the current directory that contains lines in
the format: tag filename vi-search-command
For example, if the tags file contains the line: ev mary /everywhere/
then entering the tag ev in this field would cause "vi" to read in the
file named "mary" for editing, and would position the cursor on the line
"and everywhere that ..."   [-t]
06"vi" tries to save work that was in progress when your system crashed.
To recover that work, type the name of the file here.
07Type a "y" here to see a list of files that "vi" has saved during
system crashes [-L] .
10You use a "vi" encryption option if you want to encode the
file you are creating so that other people cannot read the
file unless they know your encryption key.  You also use an
encryption option to read files that you have previously
encrypted.  When you execute the "vi" command, "vi" will
prompt you for an encryption keyword.  Use ASSIST's command
form for the "crypt" command to learn more about encrypting
files. 

The "-C" encryption option assumes that the files named are
encrypted.  [-C]
12You use a "vi" encryption option if you want to encode the
file you are creating so that other people cannot read the
file unless they know your encryption key.  You also use an
encryption option to read files that you have previously
encrypted.  When you execute the "vi" command, "vi" will
prompt you for an encryption keyword.  Use ASSIST's command
form for the "crypt" command to learn more about encrypting
files. 

The "-x" encryption option does not assume that the file
named is encrypted.  Instead, it examines the contents of
the file and tries to make an intelligent decision as to
whether or not the file is encrypted.  [-x]

000706
0107
0207
0307
0407
060700
070102030412001006
101207
121007


020537n
030732set `/bin/stty`;if test $2 -gt 1200;then /bin/echo 23;elif test $2 -gt 300;then /bin/echo 16;else /bin/echo 5;fi
071655n
102253n
122575n

0013
0005
0105
0217yYnN
0204yYnN
0205
0308
0305
0432if test ! -r tags && test ! -r /usr/lib/tags; then /bin/echo "Cannot find tags file";fi
0405
0605
0704yYnN
0705
1004yYnN
1005
1204yYnN
1205


001110
011010-c 
021011y-RY-RnN
031010-wset `/bin/stty`;if test $2 -gt 1200;then /bin/echo 23;elif test $2 -gt 300;then /bin/echo 16;else /bin/echo 5;fi
041010-t
061010-r 
071011y-LY-LnN
081110
091110
101011y-CY-CnN
111110
121011y-xY-xnN




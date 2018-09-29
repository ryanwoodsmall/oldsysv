#ident	"@(#)forms.files:sh.fs	1.8"
00000sh
00901020001LOCATION OF COMMANDS TO BE EXECUTED
01702060001FILE CONTAINING COMMANDS:
02703060030POSITIONAL PARAMETERS:
03704060001COMMAND STRING [-c]:
04705060001READ COMMANDS FROM TERMINAL [-s] (y/n):
05907020000CONTROL OF SHELL EXECUTION
06708060001INTERACTIVE SHELL [-i] (y/n):
07709060001RESTRICTED SHELL [-r] (y/n):
08710060001DISABLE FILE NAME EXPANSION [-f] (y/n):
09711060001EXECUTE SINGLE COMMAND, THEN EXIT [-t] (y/n):
10712060001EXIT ON ANY COMMAND ERROR [-e] (y/n):
11713060001TREAT UNSET VARIABLES AS AN ERROR [-u] (y/n):
12915020000SHELL DEBUGGING
13716060001PRINT INPUT LINES AS THEY ARE READ [-v] (y/n):
14717060001PRINT INPUT LINES AS THEY ARE EXECUTED [-x] (y/n):
15718060001READ COMMANDS ONLY, DO NOT EXECUTE [-n] (y/n):
16920020000ENVIRONMENT VARIABLES
17721060001MARK MODIFIED OR EXPORTED VARIABLES [-a] (y/n):
18722060001LOCATE FUNCTIONS AS THEY ARE DEFINED [-h] (y/n):
19723060001PLACE ALL KEYWORD ARGUMENTS IN ENVIRONMENT [-k] (y/n):
20725060030ADDITIONS TO THE COMMAND LINE:

01010
02030
03010-c 
04001s
06001i
07001r
08001f
09001t
10001e
11001u
13001v
14001x
15001n
17001a
18001h
19001k
20302

Depending on the options you have specified, this may be an interactive
command.  You must type control-d (^D) to exit the shell if this is so.

"sh" is the UNIX System command interpreter.  "sh"
is used to execute one or more commands from a file,
the command line, or your terminal.

01You may specify the name of a file, which contains the commands that
you want executed.  This usage of "sh" does not redirect standard input
or output, so the output of the commands will appear on your terminal.
Likewise, if any of the commands expect input from your keyboard (for
example, the "passwd" command), input will still be read from your
keyboard.  The "sh" command will exit after executing the last 
command in the file.
02Enter any command line arguments here.  The first word is set to $1,
the second to $2, etc.  This field should not be used if you specify
a value in the "COMMAND STRING" field, since all positional parameters
must be included on that line.  You might use this field if the
command file that you entered above expects arguments.
03Enter the command(s) that you want executed.  The "sh" will exit 
after executing the named command(s).  Include all arguments 
needed for the command.  Use quotes if the string contains white space 
or special characters. For example, you might enter "echo my files;ls"
here to execute the "echo" and "ls" commands.  [-c]
04Set this field to "y" if you want to enter commands from the
command line, rather than specifying them on this form, or in
a file.  If you select this option, you will remain in the shell
until you type control-d (^D).
06Setting this option has two effects:  First, hitting the <DEL> or
<BREAK> keys will not terminate the shell (use ^D for this purpose).
Second, "TERMINATE" signals (e.g. kill -15) are ignored.  If you do
not redirect standard input or output, these characteristics will
automatically be set, as well. [-i]
07If you select this option, you will not be able to do the following:
        1.  change directories
        2.  change the value of the PATH environment variable
        3.  enter any commands that start with "/"
        4.  redirect output      
Restricted shells are used when you want to control the access
a user has to the rest of your system.  [-r]
08By default, the shell allows you to use special characters when
naming files.  For example, the asterisk (*) stands for all file
names.  If you want to turn off this feature, enter a "y" here. 
If you select this option, file name expansion will be disabled
for the duration of the "sh" execution.  Selecting this option
will not turn off shell expansion on this command form.  [-f]
09Select this option if you want the shell to execute a single
command, and then exit. [-t] 
10Select this option if you want the shell to terminate any
time a command returns an error.  Normally, the shell will
continue past any errors.  This is true whether the commands
are in a file, or being entered from your terminal.   It would
make sense to select this option if the execution of each command
is somehow dependent on the successful completion of the
previous ones.  [-e]
11By default, unset variables are treated like variables with a
null value.  If you set this option, you will get an error message
("...parameter not set") when you refer to an unset variable. 
Unset variables are those for which no vale currently exists.  [-u]
13The effect of this option is to echo each input line before
executing it.  All lines are echoed, including comments.
See the "PRINT INPUT LINES AS THEY ARE EXECUTED" option for
a slightly different effect. In this way, you can see how the
shell is processing your shell script.  This is useful for debugging,
since each shell variable name will be replaced by its value.  [-v]
14The effect of this option is to echo each command before
executing it.  Each command line is preceded by a plus sign.
Comments, and other non-command lines, are not echoed.  See
the "PRINT INPUT LINES AS THEY ARE READ" option for a slightly
different effect. This is useful for debugging, since you can
see exactly which lines in the shell script are being executed,
what the values of variables are, and where errors occur.  [-x]
15If you select this option, no commands will be executed.
Each command line will be read, but will not be executed.
This option can be used to help debug your shell scripts.
It will check for shell command language syntax errors (but
not things like non-existent files or commands). [-n]
17This option should be set to "y" under the following conditions:
          1.  you have changed a variable that was inherited from
              the parent process, or created a new variable, and
          2.  you want that new value to be made available to
              child processes.    [-a]
18If you plan to invoke the same function repeatedly, you should
set this option to "y".  By selecting this option, functions are
located once when they are first defined, so need not be located 
each time they are to be executed.
By default, the shell locates functions as the are executed.
If you want the shell to locate functions when they are
defined, set this option to "y". [-h]
19When a command is executed, those variables that follow the command
name are placed in the command's environment.  By default, any
variables that precede the command are not placed in the environment.
If you want all variables, regardless of location, to be included in
the command's environment, place a "y" in this field. [-k]
20ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

010403
0203
03020401
04010306
06040915
090615
1415
15091406

0201

040546n
060836n
070935n
081046n
091152n
101244n
111352n
131653n
141757n
151853n
172154n
182255n
192361n

0105
0102
0113
013420001010
0119
0205
0216
0305
0404yYnN
0405
0604yYnN
0605
0704yYnN
0804yYnN
0904yYnN
0905
1004yYnN
1104yYnN
1304yYnN
1404yYnN
1405
1504yYnN
1505
1704yYnN
1804yYnN
1904yYnN


011110
021110
031010-c 
040001y-sY-snN
060001y-iY-inN
070001y-rY-rnN
080001y-fY-fnN
090001y-tY-tnN
100001y-eY-enN
110001y-uY-unN
130001y-vY-vnN
140001y-xY-xnN
150001y-nY-nnN
170001y-aY-anN
180001y-hY-hnN
190001y-kY-knN
201110




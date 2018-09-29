#ident	"@(#)forms.files:make.fs	1.3"
00000make
00702060101NAME OF DESCRIPTION FILE [-f]:
01704060030NAME(S) OF TARGETS TO BE MADE:
02006060030PRINT OUT THE COMPLETE SET OF MACRO
03707080101DEFINITIONS AND TARGET DESCRIPTIONS [-p](y/n):
04709060101IGNORE ERROR CODES RETURNED BY INVOKED COMMANDS [-i] (y/n):
05011060030ABANDON WORK ON THE CURRENT ENTRY BUT CONTINUE WORK
06712080101ON OTHER BRANCHES THAT DO NOT DEPEND ON THAT ENTRY [-k](y/n):
07714060101FORCE AN UNCONDITIONAL UPDATE [-u] (y/n):
08716060101PRINT COMMAND LINES BEFORE EXECUTING [-s] (y/n):
09718060101USE THE BUILT-IN RULES [-r] (y/n):
10720060101PRINT COMMANDS WITHOUT EXECUTING THEM [-n] (y/n):
11722060101USE THE COMPATIBILITY MODE FOR OLD MAKEFILES [-b] (y/n):
12024060030HAVE ENVIRONMENTAL VARIABLES OVERRIDE
13725080101THE ASSIGNMENTS MADE WITHIN THE MAKEFILE [-e] (y/n):
14027060030TOUCH THE TARGET FILES RATHER THAN
15728080101ISSUING THE USUAL COMMANDS [-t] (y/n):
16730060101USE THE DEBUG MODE [-d] (y/n):
17732060101QUESTION WHETHER THE TARGET FILE IS UP-TO-DATE [-q] (y/n):
18734060030MACRO DEFINITIONS:
19736060030ADDITIONS TO THE COMMAND LINE:

00010
01980
03030
04040
06050
07001
08060
09070
10080
11090
13100
15120
16130
17140
18970
19992

With "-f -", "make" uses standard input for the description file 
contents.  Use ^D to end standard input.  00-


"make" maintains, updates, and regenerates groups
of programs.  "make" reads a file that describes
how the components of the program depend on each
other and how to process these components to create
an up-to-date version of the program.

00The "make" description file contains commands to update one or more 
target names.  If no -f option is present, "make" tries makefile, 
Makefile, s.makefile, and s.Makefile in that order.  If you enter - in 
this field, "make" will use the standard input. [-f] 
01"make" executes the commands in the description file to create or 
update the named targets.  If no target is named on this line,
then "make" creates or updates the first target in the
description file.  The command form validation checks to see if
the target name, followed by a colon, is in the description
file.  This validation is not done if standard input (-) is used
for the description file. 
03If you select this option, then "make" will print all active macro 
definitions, including environmental variables and macros defined in 
your description file.  Also, "make" will show you all the dependencies 
for your target names, and how "make" handles certain file name 
suffixes, such as .h and .c.   [-p]
04"make" normally stops if any command signals an error by returning a 
nonzero error code.  These errors will be ignored by "make" if you set 
the -i option.  You can also have "make" ignore these signals by 
putting the fake target name ".IGNORE" in your description file. [-i]
06If you set the -k option, "make" will not stop if the current entry 
fails.  Instead, "make" will stop working on the current entry, and 
will continue working on any branches that do not have dependencies on 
the entry that failed.  [-k]
07"make" updates a target only if the files on which the target is 
dependent are newer than the target.  Use the -u option if you want to 
force the target to be updated even if the dependents are not newer than 
the target. [-u] 
08"make" usually prints the command it is about to execute.  Using the 
-s option causes "make" to be in what is sometimes called the silent 
mode; with -s set, you will not see the commands that are being 
executed by "make."  [-s]
09One of "make"'s sources of information is a set of built-in rules.  For 
example, "make" knows that a file ending in ".y" is a "yacc" file; 
"make" will take this file and run "yacc" on it even if the command 
"yacc" is never named in the description file.  If you want to use "make"
without its built-in rules, you should answer "n" in this field. [-r]
10This is the no-execute mode.  With the -n option, you can see 
what commands "make" will execute without actually executing 
those commands. [-n]
11The -b option allows old makefiles (makefiles written for the old
version of "make") to run with the current version of "make". [-b]
13In the "make" command, the assignment of macros in the description file 
overrides assignment of macros in the environment.  Use the -e option 
if you want environmental macro definitions to override macro 
definitions in the description file.   [-e]
15You can use the -t (touch) option to save time.  It will cause "make" 
to treat the relevant files as up-to-date, and thus avoid superfluous 
compilations.  For example, you might use the -t option if all you had 
done was add comments to a header file.  Obviously, you should use this 
option with caution since it subverts the intention of "make" and 
destroys "make"'s memory of previous relationships.   [-t]
16The debugging option cause "make" to print out a very detailed
and verbose description of what it is doing.  The -d option is
available only if the "make" command on your system was compiled
with -dmkdebug.  [-d]
17"make" with the -q (question) option returns a zero or non-zero status 
code depending on whether the target file is or is not up-to-date.   [-q]
18"make" allows you to put macro definitions in the command line.  These 
command-line macros override corresponding macro definitions found in 
the description file.  The validation on this field checks if there is an
"=" sign in your macro definition.
19ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



000237if test -r makefile; then echo "makefile"; elif test -r Makefile; then echo "Makefile"; elif test -r s.makefile; then echo "s.makefile"; elif test -r s.Makefile; then echo "s.Makefile"; fi
030755n
040966n
061270n
071448n
081655y
091841y
102056n
112263n
132561n
152847n
163037n
173265n

0015-
0032if test -d $F00; then echo "$F00 IS A DIRECTORY";fi
0132i=$F00; j=$F01; if test "$i" = "-"; then exit; fi; /usr/bin/egrep "^[a-zA-Z0-9./\].*[ 	]$j.*:|^$j.*:"  $i > /dev/null; if test $? -ne 0; then echo "$j NOT A TARGET NAME IN $i"; fi
0304yYnN
0404yYnN
0604yYnN
0704yYnN
0735
0804nNyY
0904nNyY
1004yYnN
1104yYnN
1304yYnN
1504yYnN
1604yYnN
1704yYnN
1832if test -z "`echo $F18 | grep =`"; then echo "$F18 DOES NOT CONTAIN AN = SIGN";fi


001010-f if test -r makefile; then echo "makefile"; elif test -r Makefile; then echo "Makefile"; elif test -r s.makefile; then echo "s.makefile"; elif test -r s.Makefile; then echo "s.Makefile"; fi
011110
021110
031011y-pY-pnN
041011y-iY-inN
051110
061011y-kY-knN
070001y-uY-unN
081011n-sN-syY
091011n-rN-ryY
101011y-nY-nnN
111011y-bY-bnN
121110
131011y-eY-enN
141110
151011y-tY-tnN
161011y-dY-dnN
171011y-qY-qnN
181110
191110




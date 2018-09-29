#ident	"@(#)forms.files:Cflist.fs	1.5"
400Command Forms
00101080101........... Create and maintain groups of files in one archive file
01102090101.......... Pattern scanning and processing language
02103090000.......... Scan a big file
03104090101.......... Show the calendar for a specified year
04105090101.......... Concatenate and show files
05106080101........... C compiler
06107080101........... Change working directory
07108110101........ Change permissions of files and directories
08109110101........ Change owner of files and directories
09110090101.......... Show byte and line number of differences
10111100101......... Select/reject common lines
11112080101........... Copy files
12113100101......... Copy file archives in and out of a file
13114110101........ Encode or decode a file
14115120101....... Split a file into sections according to an expression
15116080101........... Call up another UNIX system
16117090101.......... Cut out selected fields of each line of a file
17118100101......... Show and set the date and time
18119080101........... Report number of free disk blocks
19120100101......... Show all lines that differ
20121120101....... Compare directories
21122080101........... Summarize dis usage
22123100101......... Write arguments to the standard output
23124080101........... Edit files with a line oriented text editor
24125110101........ Search files using a regular expression
25126090101.......... Report or set environment for command execution
26127110101........ Search files for strings
27128100101......... Report file type
28129100101......... Find files
29130100101......... Search files for lines matching a pattern
30131080101........... Print user and group IDs names
31132100101......... Terminate a process
32133080101........... Link editor for common object files
33134090000.......... Generate code for a lexical analysis to text
34135080101........... Send files to a line printer
35136080101........... Link files, give a file more than one name
36137120101....... Report the status of the line printer
37138080101........... List contents of directory
38139100101......... Send mail to user or read mail
39140100101......... Maintain, update, and regenerate groups of programs
40141100101......... Report/change permissions for others to "write" to you
41142110101........ Make a directory
42143080101........... Format text with MM macros using "nroff" 
43144080101........... Move/rename files
44145110101........ Run a command immune to hangups and quits
45146110101........ Format text containing "nroff" commands
46147080101........... Dump an object file in ASCII, binary, hex, or octal
47148120101....... Change login password
48149110101........ Paste files together by columns
49150080101........... Examine files on screen-full at a time
50151080101........... Print files on standard output
51152080101........... Report status of currently executing commands
52153090101.......... Print working directory name
53154080101........... Remove files and directories
54155110101........ Remove empty directories
55156090101.......... Symbolic debugger for C and Fortran
56157090101.......... Stream editor
57158080101........... Execute commands 
58159100101......... Print section sizes of common object files
59160100101......... Sort and/or merge the lines of files
60161110101........ Find spelling errors
61162110101........ Split a file into pieces by number of lines
62163110101........ Strip symbol and line numbers from object files
63164100101......... Report or set options for a terminal
64165080101........... Become super user or anther user
65166100101......... Set tab stops on your terminal
66167100101......... Show last part of a file
67168110101........ Set file creation permissions
68169110101........ Show name of current UNIX system
69170100101......... Copy files from one UNIX system to another UNIX system
70171080101........... Edit files with a screen oriented text editor
71172080101........... Report number of characters, words, and lines in a file
72173090101.......... Show who is on the system
73174100000......... Yet another compiler compiler

The Command Forms menu lists all of the implemented command
forms for Release 1.0 of ASSIST.

00"ar" maintains groups of files combined into a single
archive file.  "ar" is primarily used to create and
update archive files of object code for library files;
for example, files such as /lib/libc.a and /lib/libm.a
were created with "ar".  You can also use "ar" to
combine a set of text files into a single archive file.
01"awk" is a programmable UNIX system filter that
does pattern scanning and language processing.  It
includes text substitution, arithmetic, variables,
built-in functions, and a programming language that
looks similar to C. 
02"bfs" is used to scan files.  It is similar to "ed"
except that it is read-only and can process much
larger files.  Files can be up to 1024K bytes
(maximum).  "bfs" is usually more efficient than
"ed" for scanning a file, since the file is not
copied to a buffer.  It is most useful for identi-
fying sections of a large file where "csplit" can
be used to divide it into more manageable pieces.
03"cal" prints a calendar on your terminal.  If you
specify neither a month nor a year, "cal" will print
a calendar for the current month in the current year.
04"cat" prints the content of files on your terminal.
If you specify multiple files, "cat" prints each
in turn, thus concatenating the multiple files.
05"cc" is the interface for the C Compilation System.
The compilation tools consist of a preprocessor,
compiler, optimizer, assembler, and link editor.
The "cc" command processes the supplied options
and then executes the various tools with the proper
arguments.  In typical usage, "cc" compiles source
files whose names end with ".c" and produces an
executable object file, "a.out".
06"cd" allows you to change (that is, move to a different)
working directory.  This is the command to use for moving
around in the UNIX system hierarchical file system.  The "cd"
command form will allow you to change directories within
ASSIST, however, when you exit ASSIST, you will be in the
directory from which you invoked ASSIST.  While in ASSIST,
you may also change directories via the Pop-up menu (^F).
07"chmod" changes the permissions of files.  You can
use it in symbolic or absolute mode.  In the symbolic
mode, you specify the permission by using a string
that indicates which permission should change; what
type of permission is involved; and whether this
permission is to be added, deleted, or set.  In the
absolute mode, you specify the permission by a three-
or four-digit octal code.
08"chown" changes the owner of files and directories.
You name the new owner of file or directory using
the person's login name or numeric user ID.  You
must own the file or directory, or you must be
super user to use "chown".
09"cmp" is used to compare two files.  By default,
"cmp" produces no output if the files are identical.
If they differ, "cmp" lists the byte and line number
of the first difference.
10"comm" prints lines that are either unique or common
to two files.  The files must be sorted in ASCII
collating order.  Use the "sort" command to do this.
By default, three columns of output are produced.
The left column contains those lines in the first
file that are not in the second.  The middle column
contains those lines that are in the second file,
but not the first.  The right column contains those
lines common to both files.
11"cp" copies the named input file to the target
output file.  Under no circumstances can the input
file and the target file be the same.  If the target
is a directory, then one or more files are copied to
that directory.  If the target is an existing file,
its contents are destroyed.  If the target is not an
existing file, a new file is created.  The input file
cannot be a directory.  "cp" differs from "mv" because
"cp" duplicates the file in the new location without
removing it from its current location.  "mv" actually
removes the file/directory from its current location.
12"cpio" has three distinct uses.  First, "cpio -o"
can take a list of file names, and display their
content on standard output.  By redirecting output,
you can create an archive and write it to external
storage on to a single file.  Second, "cpio -i" can
take an archive file coming from standard input,
and unarchive it into its constituent files.  This
form is often used to read in a tape made with an
earlier "cpio" command.  Finally, "cpio -p" can copy
a set of files and directories from one location to
another.
13"crypt" converts readable text into encrypted text
or unencrypts text that you encrypted using "crypt".
By default, input comes from standard input, and
output goes to standard output.
14"csplit" splits a file into smaller files, based on
a set of splitting criteria specified by the user.
The first file created contains everything up to
(but not including) the first splitting criterion,
the second file contains everything from the first
criterion up to the second, and so forth.  In general,
the "csplit" command will try to create one more file
than the number of splitting criteria you specify.
15"cu" is used to call up and log in to another terminal
or computer system.  In order to use "cu" to call up
another computer system, you must have a login name on
that system.  You can also use "cu" to transfer files
between your system and another UNIX system.
16"cut" prints selected columns or fields from a file.
If the material you want appears in the same columns
on every line of the file, cut based on columns.  If
the material you want is separated by some character
(for example, <TAB>), cut by field.
17"date" prints the day of the week, date, and time.
If you would like to print this information in a
different way, "date" enables you to change the
format.  If you have super user permissions for
your computer system, you can also change the date
using the "date" command.
18"df" shows the number of free blocks and free
i-nodes available on your system.
19"diff" tells you what lines you must change to
make two files identical.  "diff" can also create
scripts that you can use with "ed" to change the
files.
20"dircmp" compares two directories and lists all
the files in them stating whether they are the
same or different.  Files are marked as the same
if they have the same name and identical contents.
21"du" is used to obtain the number of disk blocks
used by files and directories.  If you specify no
directories or files, "du" will list the usage,
the current directory, and any sub-directories.
22"echo" prints strings on your terminal.  One use
of "echo" is to find out the current values of your
environment variables.  For example, to find out your
current login directory, you could enter "echo $HOME".
23"ed" is a line-oriented text editor that allows you
to create and edit files.  Editing with "ed" is
line-by-line rather than full-screen (as in "vi").
You can use "ed" on both hard-copy and video display
terminals.
24"egrep" searches for a pattern in a file, using
full regular expressions.  "egrep" works like
"fgrep" and "grep".  Of these three, "egrep" is
the slowest but also the most powerful.
25"env" is used in two different ways.  First, it can
be used to list all of your environment variables
and their current values.  The second use of "env"
is to execute a command with new values for your
environment variables.
26"fgrep" searches for a pattern in a file using
strings.  It searches only for lines that match
the strings exactly, but allows you to specify
many exact patterns to be searched for simultane-
ously.  "fgrep" works like "grep" and "egrep",
but it is the least powerful of the three.
27"file" gives you the file type for the file names
that you list on the command line.  An example of
a file type is an ASCII character file.
28The purpose of "find" is to locate one or more files that satisfy 
conditions that the user specifies.  In the simplest case, the 
condition is that of having a specified name (e.g., assist.c).  
More complicated cases include, for example: the file must be
newer than a specified file, must have a specific permission
pattern, or must have been modified prior to a specified date.
29"grep" searches for a pattern in a file using a
limited regular expression.  "grep" works like
"fgrep" and "egrep", and it is in between "fgrep"
and "egrep" in terms of speed and power.
30"id" returns your user id and group id.  These ids
are shown in both their mnemonic and numeric forms.
"id" requires no input.
31"kill" terminates processes.  It does this by
sending a signal number to the specified processes.
By default, "kill" sends signal number 15 to kill
a process.
32"ld" is a link editor that combines several object
files into one, performs relocation, resolves
external symbols, and supports symbol table infor-
mation for symbolic debugging.  In the simplest case,
the names of several object programs are given, and
"ld" combines them, producing an object module that
can either be executed or used as input for a sub-
sequent "ld" run.  The output of "ld" is left in
"a.out".  By default this file is executable if no
errors occurred during the load.  If any input file
is not an object file, "ld" assumes it is either
an archive or library or text file containing link
editor directives.  Normally, "ld" is not invoked
directly by the user, but by the compiler, "cc".
33
"lex" generates programs for simple lexical analysis 
of text.  "lex" produces a C code subroutine as output
that you compile and combine with other programs.  You 
can use the processing done by "lex" as the first step 
in creating a compiler-type program.  In addition, you
can use output from "lex" as a preprocessing tool for
many different software generation functions.  
34"lp" sends files to a line printer.  If you do not
include any file names, "lp" will take input from
standard input.
35"ln" links the file(s) to a new file name on to
files by the same name in a different directory.
"ln" does not make copies of the files, it simply
enables you to access or change the contents of a
file by more than one name.  If you change the
file using either file name, the file will be
changed when you access it using either name.
You can delete the new file name, but the original
file will still exist.  If the new file name is an
existing file, its contents are destroyed.
36"lpstat" prints information about the current status
and configuration of the line printers on your system.
You may request multiple pieces of information at one
time.
37"ls" lists contents of a directory or directories,
and can give detailed information about each file
listed.

The "ls" menu lists two versions of the "ls" command
form; a short "typical" use of "ls", and a complete
version that incorporates all of the options.
38"mail" prints your mail, message-by-message.  When
you login, you receive a message if you have mail.
By using the options, you can change the way your
mail is printed.  You may also send mail to people
on your system and other computer systems.  To do
this, you use the "persons" argument which names
the recipients of the mail.
39"make" maintains, updates, and regenerates groups
of programs.  "make" reads a file that describes
how the components of the program depend on each
other and how to process these components to create
an up-to-date version of the program.
40"mesg" can be used in two ways.  First, you can
use it to prevent or allow users to "write" to
your terminal.  Second, it can tell you whether
users are currently able to "write" to your
terminal.
41"mkdir" creates directories.  When it creates a
directory, it assigns read, write and execute
permissions to all those who have access to the
directory.  You must have write permission in a
directory to create sub-directories in it.
42"mm" invokes the "nroff" text formatter.  The "nroff"
text formatter is a program that formats text by
interpreting special requests and macros that you
place in a file.  You can use the "mm" command to
format a variety of documents such as letters and
reports.  To use "mm" you must have first created
a file that contains MM macros.
43"mv" moves an input file(s) or a directory to the
target or different file or directory.  If the input
is a directory, then one or more files in the direc-
tory are moved to that directory.  "mv" differs from
"cp" because it actually removes the file or directory
from its current location and puts it into the new
location.  "cp" copies the file in the new location
without removing it from its current location.  If you
"mv" a file to an existing file, the contents of the
existing file are destroyed.
44"nohup" executes the commands, and prevents them
from terminating if it receives a "hangup" or "quit"
signal.  You might use this command if you wanted to
execute a command and log off before the command
completed.  Normally, when you log off, any
currently running commands are terminated.
45"nroff" is a text formatter that manipulates text
by interpreting special requests and macros that
you place in a file.  You can use "nroff" to format
a variety of documents such as letters, reports,
and books.  To use the "nroff" command, you must 
create a text file that contains "nroff" requests.
46"od" dumps a file in one or more formats according
to the option you pick.  If you do not use an option,
"od" prints in octal words.  If you do not provide a
file name, "od" uses standard input.  If you want to
start reading your file at some location other than
the beginning of the file, the FILE OFFSET OPTIONS
allow you to advance your starting point.
47"passwd" changes your password or puts a new
password in place for your login name.  You will
be prompted for your current password, before it
prompts you for your new password.  Passwords must
have the following characteristics:
 
  1. must be at least 6 characters long
  2. must contain at least 2 alphabetic characters
     and at least 1 numeric or special character
  3. must differ from your login name

Ordinary users may change only the password which
corresponds to their login name.  If you have super
user permission for your system, you may change any
password.  In the event you forget your password,
the system administrator can give you access to your
login again.
48"paste" concatenates corresponding lines of input
files.  It treats each file as a column or columns
of a table and pastes them together horizontally
or vertically.
49"pg" allows you to examine a file one screenful at
a time.  Each screenful is followed by a prompt.
If you type a carriage return, another page is
displayed.
50"pr" prints files on your terminal.  By default,
the file is printed separated into pages.  Each
page is headed by a page number, the date and
time, and the name of the file.
51"ps" reports the status of processes that are
running on your system.  Without options, "ps"
prints information on processes associated with
your terminal.  With options, "ps" can report on
the status of processes that were started by other
users on your system.
52"pwd" prints the path name of the current working
directory.
53"rm" removes files.  To remove directories, and the
files and sub-directories in the directories, you
must use the "-r" option.
54"rmdir" is the remove directory command.  "rmdir"
will remove only empty directories.  To use "rmdir"
you must own a directory and have write permission
for it.  If you want to remove directories that
contain files and sub-directories, you must use
the "rm" command.
55"sdb" is a debugger for C and Fortran77 programs.
You can use "sdb" to examine "core" images of
programs that aborted at runtime and to monitor
and control program execution.
56"sed" reads lines from a file, changes the lines
according to a script you supply, and puts out the
modified lines on standard output.  The actual
files are not changed.
57"sh" is the UNIX System command interpreter.  "sh"
is used to execute one or more commands from a file,
the command line, or your terminal.
58"size" prints size information for each section of
an object file.  The size of the text, data, and
bss (uninitialized data) sections are printed along
with the total size of the object file.  If you
use an archive file as input to the command, the
information for all archive members is displayed.
59"sort" enables you to sort the lines of files, merge
files, or check if files are sorted properly.  "sort"
can sort a file according to fields.  "sort" can sort
the lines of files in many different orders including
reverse alphabetical.

The "sort" menu lists two versions of the "sort"
command form; a short typical use of "sort", and a
complete version that incorporates all the options.
60"spell" collects words from specified files and looks
them up in a spelling list.  Words that neither occur
among nor are derivable (by applying certain inflec-
tions, prefixes, and/or suffixes) from words in the
spelling list, are printed on the standard output.
"spell" ignores most macros used with UNIX system
text formatting commands (for example, "eqn," "mm,"
"nroff," "tbl," and "troff").
61"split" divides a file into a number of smaller files.
The first "n" lines of the input file are placed in
the first output file, the second "n" lines are placed
in the second, etc.  By default, "n" is 1000 lines,
but you may specify a different value.  The input file
is not changed.
62"strip" removes the symbol table and line number
information from common object files, including
archives.  Once this has been done, no symbolic
debugging access will be available for that file.
Therefore, "strip" is normally run only on
production modules that have been debugged and
tested.
63"stty" sets many of your terminal options.  Use
"stty" without arguments to see the current set-
tings of certain options, such as the baud rate
and the current erase characters.  Use "stty -a"
to see the current settings of all options.  You
can change an option setting using "stty" with
the option name and a new value.
64"su" allows you to re-login, without terminating
your current session.  This is useful if you want to
log in as a different user, execute a few commands,
and then return to your current session.  In effect,
this allows you to change your identity to a different
login name.
65"tabs" can be used by you to set the tab stops on
your terminal.  A number of different tab settings
are pre-defined.  You may also define your own tab
settings.  In order to use "tabs", your terminal
must have the ability to remotely set its hardware
tabs.
66"tail" is used to print the end of a file on your
terminal.  Using the options to "tail", you may
specify just how much of the file contents are
printed.  By default, "tail" prints the last 10
lines of a file.
67"umask" is used to remove permissions when a new
file is created.  It uses the same "absolute" mode
as the "chmod" command.  Thus, 022 will remove
write permission for "group" and "others".
68"uname" prints the name of the UNIX system you are
using.  Besides determining the system name, you
can retrieve other information such as the name
that the system is known by to a communication
network, system release and version, and machine
hardware name.
69"uucp" (UNIX system to UNIX system copy) copies
files from one UNIX system to another.  You may
send files from the current system to a remote
system, from a remote system to this system,
between two remote systems, or from one location
to another on this system.
70"vi" is a full-screen text editor.  When you use
"vi", the changes you make are reflected on the
screen as you make them.  You can use "vi" to create
and edit any type of text file.  Examples of text
files include:  (1) mail messages, (2) documents
that will be formatted by a command such as "mm",
(3) programs in languages such as C, and (4) shell
scripts.  
71"wc" counts the number of lines, words, and/or
characters in a file.  Without any options,
"wc" lists all three counts.
72"who" gives you information about your system and
the users on your system.  By default, "who" prints
the names of users who are currently logged in to
your system, their terminal numbers and the time
that they logged in.  You can get information on the
system or you can change user information you get.
73
"yacc" is a utility for creating parser subroutines.  
It is primarily used in the generation of software
compilers.  "yacc" produces C source code for a parser 
subroutine; the parser subroutine is then compiled and 
finally used with a program that calls the subroutine
to parse input.  

000105ar
010205awk
020305bfs
030405cal
040505cat
050605cc
060705cd
070805chmod
080905chown
091005cmp
101105comm
111205cp
121305cpio
131405crypt
141505csplit
151605cu
161705cut
171805date
181905df
192005diff
202105dircmp
212205du
222305echo
232405ed
242505egrep
252605env
262705fgrep
272805file
282905find
293005grep
303105id
313205kill
323305ld
333405lex
343505lp
353605ln
363705lpstat
373805ls
383905mail
394005make
404105mesg
414205mkdir
424305mm
434405mv
444505nohup
454605nroff
464705od
474805passwd
484905paste
495005pg
505105pr
515205ps
525305pwd
535405rm
545505rmdir
555605sdb
565705sed
575805sh
585905size
596005sort
606105spell
616205split
626305strip
636405stty
646505su
656605tabs
666705tail
676805umask
686905uname
697005uucp
707105vi
717205wc
727305who
737405yacc

00ar
01awk
02bfs
03cal
04cat
05cc
06cd
07chmod
08chown
09cmp
10comm
11cp
12cpio
13crypt
14csplit
15cu
16cut
17date
18df
19diff
20dircmp
21du
22echo
23ed
24egrep
25env
26fgrep
27file
28find
29grep
30id
31kill
32ld
33lex
34lp
35ln
36lpstat
37ls
38mail
39make
40mesg
41mkdir
42mm
43mv
44nohup
45nroff
46od
47passwd
48paste
49pg
50pr
51ps
52pwd
53rm
54rmdir
55sdb
56sed
57sh
58size
59sort
60spell
61split
62strip
63stty
64su
65tabs
66tail
67umask
68uname
69uucp
70vi
71wc
72who
73yacc


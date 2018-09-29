#ident	"@(#)forms.files:tail.fs	1.3"
400tail
00901270030TAIL MENU
01704210030 ....... Typical usage
02706240030 .... Follow file contents

"tail" is used to print the end of a file on your
terminal.  Using the options to "tail", you may
specify just how much of the file contents are
printed.  By default, "tail" prints the last 10
lines of a file.  In its usual usage, tail prints
the desired information, and then exits.  With the
"-f" option, it becomes an interactive command.

01"tail" is used to print the end of a file on your
terminal.  Using the options to "tail", you may
specify just how much of the file contents are
printed.  By default, "tail" prints the last 10
lines of a file.
02"tail" has a "follow" mode, invoked by the
"-f" option.  Here "tail" will print the last
10 lines of a file, and then wait.  As more
information is written to the file, "tail" will
print it out.  This usage of the "tail" command
is useful when a program is writing to a file,
since this allows you to follow the progress of
the program.  You must hit the BREAK key to 
terminate "tail" when used in this way.

010416tail
020616tail -f

01tailn
02taili


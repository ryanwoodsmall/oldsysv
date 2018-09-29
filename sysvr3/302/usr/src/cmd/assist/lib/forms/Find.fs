400FIND FILES
00001280000FIND FILES
01703180101..... Write arguments to the standard output
02704190101.... Search files using a regular expression
03705190101.... Search files for strings
04706180101..... Find files
05707180101..... Search files for lines matching a pattern
06708160101....... List contents of directory

The Find Files menu lists commands that you use to
search for files or search for files that contain
particular patterns.

01"echo" prints strings on your terminal.  One use
of "echo" is to find out the current values of your
environment variables.  For example, to find out your
current login directory, you could enter "echo $HOME".
02"egrep" searches for a pattern in a file, using
full regular expressions.  "egrep" works like
"fgrep" and "grep".  Of these three, "egrep" is
the slowest but also the most powerful.
03"fgrep" searches for a pattern in a file using
strings.  It searches only for lines that match
the strings exactly, but allows you to specify
many exact patterns to be searched for simultane-
ously.  "fgrep" works like "grep" and "egrep",
but it is the least powerful of the three.
04The purpose of "find" is to locate one or more files that satisfy 
conditions that the user specifies.  In the simplest case, the 
condition is that of having a specified name (e.g., assist.c).  
More complicated cases include, for example: the file must be
newer than a specified file, must have a specific permission
pattern, or must have been modified prior to a specified date.
05"grep" searches for a pattern in a file using a
limited regular expression.  "grep" works like
"fgrep" and "egrep", and it is in between "fgrep"
and "egrep" in terms of speed and power.
06"ls" lists contents of a directory or directories,
and can give detailed information about each file
listed.

The "ls" menu lists two versions of the "ls" command
form; a short "typical" use of "ls", and a complete
version that incorporates all of the options.

010313echo
020413egrep
030513fgrep
040613find
050713grep
060813ls

01echo
02egrep
03fgrep
04find
05grep
06ls


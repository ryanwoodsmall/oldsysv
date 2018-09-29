#ident	"@(#)forms.files:Stats.fs	1.2"
400STATISTICS
00001150000STATISTICS OF FILES, DIRECTORIES, AND DISK BLOCKS
01703130101.... Report number of free disk blocks
02704130101.... Summarize disk usage
03705150101.. Report file type
04706130101.... List contents of directory with options for file data
05707150101.. Print size of object files
06708130101.... Report numbers of characters, words, and lines in file

The Statistics of Files, Directories, and Disk Blocks
menu lists commands that provide information on disk
space and usage, file type and size, and directory
content.

01"df" shows the number of free blocks and free
i-nodes available on your system.
02"du" is used to obtain the number of disk blocks
used by files and directories.  If you specify no
directories or files, "du" will list the usage
the current directory and any sub-directories.
03"file" gives you the file type for the file names
that you list on the command line.  An example of
a file type is an ascii character file.
04
"ls" lists contents of a directory or directories,
and can give detailed information about each file
listed.

The "ls" menu lists two versions of the "ls" command
form; a short "typical" use of "ls", and a complete
version that incorporates all of the options.
05
"size" prints size information for each section of
an object file.  The size of the text, data, and
bss (uninitialized data) sections are printed along
with the total size of the object file.  If you
use an archive file as input to the command, the
information for all archive members is displayed.
06"wc" counts the number of lines, words, and/or
characters in a file.  Without any options,
"wc" lists all three counts.

010310df
020410du
030510file
040610ls
050710size
060810wc

01df
02du
03file
04ls
05size
06wc


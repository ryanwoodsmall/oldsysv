#ident	"@(#)forms.files:Create.fs	1.2"
400CREATE
00001140000CREATE, COPY, RENAME, AND REMOVE FILES
01703130101............. Concatenate and show files
02704120101.............. Copy files
03705120101.............. Edit files with a line oriented text editor
04706120101.............. Link files, give a file more than one name
05707120101.............. Move/rename files
06708120101.............. Remove files
07709120101.............. Edit files with a screen oriented text editor
08710230101... Brief introduction to the "vi" screen editor

The Create, Copy, Rename and Remove Files menu lists
commands that enable you to make copies of files,
link two filenames to one file, and remove files and
directories.

01"cat" prints the content of files on your terminal.
If you specify multiple files, "cat" prints each
in turn, thus concatenating the multiple files.
02"cp" copies the named input file to the target
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
03"ed" is a line-oriented text editor that allows you
to create and edit files.  Editing with "ed" is
line-by-line rather than full-screen (as in "vi").
You can use "ed" on both hard-copy and video display
terminals.
04"ln" links the file(s) to a new file name on to
files by the same name in a different directory.
"ln" does not make copies of the files, it simply
enables you to access or change the contents of a
file by more than one name.  If you change the
file using either file name, the file will be
changed when you access it using either name.
You can delete the new file name, but the original
file will still exist.  If the new file name is an
existing file, its contents are destroyed.
05"mv" moves the named input file(s)/directory to the
"mv" moves an input file(s) or a directory to the
target or different file or directory.  If the input
is a directory, then one or more files in the direc-
tory are moved to that directory.  "mv" differs from
"cp" because it actually removes the file or directory
from its current location and puts it into the new
location.  "cp" copies the file in the new location
without removing it from its current location.  If you
"mv" a file to an existing file, the contents of the
existing file are destroyed.
06"rm" removes files.  To remove directories, and the
files and sub-directories in the directories, you
must use the "-r" option.
07"vi" is a full-screen text editor.  When you use
"vi" is a full-screen text editor.  When you use
"vi", the changes you make are reflected on the
screen as you make them.  You can use "vi" to create
and edit any type of text file.  Examples of text
files include:  (1) mail messages, (2) documents
that will be formatted by a command such as "mm",
(3) programs in languages such as C, and (4) shell
scripts.  
08ASSIST's "vi" walkthru is designed to get you started
with a basic set of "vi" editing commands.

"vi" is a full-screen text editor.  When you use
"vi", the changes you make are reflected on the
screen as you make them.  You can use "vi" to create
and edit any type of text file.  Examples of text
files include:  (1) mail messages, (2) documents
that will be formatted by a command such as "mm",
(3) programs in languages such as C, and (4) shell
scripts.  

010309cat
020409cp
030509ed
040609ln
050709mv
060809rm
070909vi
081009assist viwalk

01cat
02cp
03ed
04ln
05mv
06rm
07vi
08viwalk


#ident	"@(#)forms.files:chmod.fs	1.2"
400chmod menu
00103250130
01105250130

"chmod" changes the permissions of files.
The chmod menu lists two versions of the chmod command for which muse
has command forms.  These versions correspond to the two modes in which
"chmod" can be used.  In the symbolic mode, the permission is specified
by a string specifying whose permission is to be changed, what type of 
permission is involved, and whether this permission is to be added, 
deleted, or set.  In the absolute mode, the permission is specified by
a three- or four-digit octal code.
You can obtain the permissions of a file by executing 
"ls -l <filename>" from the UNIX System command level [use the ASSIST
^E command].

00In the symbolic mode, the mode is specified by a string that has a
"who" part, a "how" part and a "what" part.  In the "who" part,
you specify whose permission is to be changed. In the "how" part,
you specify how the permission is to be changed.  In the "what"
part, you specify which permission(s) are to be changed.  Single-letter
abbreviations are used.
01In the absolute mode, you directly specify the permission pattern
as a string of three or four octal numbers.  These numbers stand for a
12 or 16-bit string, where each bit corresponds to a particular
permission.  For example, the righmost bit stands for execute
permission for "others".

000323Symbolic mode of chmod
010523Absolute mode of chmod

00chmods
01chmoda



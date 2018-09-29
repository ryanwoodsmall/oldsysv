#ident	"@(#)forms.files:cmp.fs	1.5"
00000cmp
00702060101NAME OF FIRST FILE:
01704060101NAME OF SECOND FILE:
02706060001LIST ALL DIFFERENCES [-l] (y/n):
03708060001RETURN ONLY EXIT CODE [-s] (y/n):
04710060030ADDITIONS TO THE COMMAND LINE:

00030
01040
02011l
03020-s
04052

By entering a hyphen instead of a file name, you have indicated that
you will supply input from standard input.  To indicate the end of
your input (and return to this form), type ^D. 00-

"cmp" is used to compare two files.  By default,
"cmp" produces no output if the files are identical.
If they differ, "cmp" lists the byte and line number
of the first difference.

00Enter here the name of the first file to be compared.  
01Enter here the second file to be compared.  
02By default, "cmp" will report only the first difference between the files.
If you want to see all differences, enter a "y" in this field.  For
each difference, you will get a decimal starting byte number and 
the octal representation of the differing bytes in each file.  [-l]
03If you want the command return codes only, select this option.
If you do select this option, you will not receive any information
on differences between the files.  Return codes are:
   0=identical  1=differ  2=error
You can look at the return code of this command by entering 
   ; echo "$?"       on the "ADDITIONS TO THE COMMAND LINE" field.  [-s]
04Anything typed here will be appended to the command line.
This field can be used for "piping," "redirection," or to
execute a command in the "background."  However, since no
error checking is performed on this field, it should be
used with caution.

0203
0302


020639n
030840n

0015-
0102
013410001010
0107
0204yYnN
0205
0304yYnN
0305


001110
011110
020001y-lY-lnN
031011y-sY-snN
041110




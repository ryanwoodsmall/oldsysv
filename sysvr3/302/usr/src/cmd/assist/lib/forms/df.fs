#ident	"@(#)forms.files:df.fs	1.4"
00000df
00001060000NAME(S) OF FILE SYSTEMS [LEAVE BLANK FOR
01702080030A REPORT OF FREE SPACE ON ALL SYSTEMS] :
02704060001ONLY REPORT LOCAL FILE SYSTEMS [-l] (y/n) :
03706060101REPORT TOTAL ALLOCATED BLOCK FIGURES [-t] (y/n) :
04708060101ONLY REPORT ACTUAL COUNT OF BLOCKS IN THE FREE LIST [-f] (y/n) :
05710060030ADDITIONS TO THE COMMAND LINE: 

01050
02020
03030-t
04042f
05062


The "df" command prints out the number of free blocks and free i-nodes 
available for mounted file systems.  

01"df" will show you the free space on all mounted systems if you leave 
this field blank.  You can ask for information about the free space 
available on specific systems by naming either the mounted directory 
name (e.g., /tmp) or the device name (e.g., /dev/dsk/c1d0s2).  
02By default, "df" reports the free space on all locally and
remotely mounted file systems.  If you say "y" here, then "df"
will report on only the locally mounted file systems. [-l]
03If you type "y" here, "df" will also show the figures for the
total allocated blocks and i-nodes. [-t]
04If you type "y" here, "df" will report the actual count of the
blocks in the free list, rather than taking the figure from
the super-block. [-f]
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not do any error checking on this field,
you should use it with caution.



020450n
030656n
040871n

0132F=$F01;if test `uname -r | cut -c1` -ge 3; then if test "`echo $F | cut -c1`" = "/"; then if test ! -r $F -a ! -d $F; then echo "CANNOT OPEN $F"; exit; fi; F=/`echo $F | cut -f2 -d'/'`; fi; fi; /etc/mount|egrep "[ (]$F[ )]|^$F ">/dev/null; if test $? -ne 0;then echo "$F NOT A MOUNTED FILE SYSTEM"; exit;fi; if test "$F02" = "y" -o "$F02" = "Y"; then /etc/mount|grep "$F.*remote">/dev/null; if test $? -eq 0; then echo "$F NOT A LOCALLY MOUNTED FILE SYSTEM";fi; fi
0204yYnN
0235
0317yYnN
0304yYnN
0404yYnN
0432if test "$F04" = "y" -o "$F04" = "Y"; then /etc/mount|grep "$F01.*remote">/dev/null; if test $? -eq 0; then echo "-f OPTION DOES NOT PRINT INFO FOR REMOTE MOUNTED SYSTEMS ";fi; fi


011110
021011y-lY-lnN
031011y-tY-tnN
041011y-fY-fnN
051110




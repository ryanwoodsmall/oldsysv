#ident	"@(#)forms.files:cd.fs	1.4
00000cd
00701060001NAME OF NEW WORKING DIRECTORY:
01703060050ADDITIONS TO THE COMMAND LINE:

00010echo $HOME
01022


"cd" allows you to change -- i.e., move to a different -- working 
directory.  This is THE command to use for moving around in the 
hierarchical UNIX file system.
It performs the same function as Switching Directories in the Popup.

00Type name of directory you want to move to (default: home directory).
You can provide: (1) a full pathname, starting with a "/"; 
(2) a subdirectory of the current directory (no pathname needed);
(3) a subdirectory of any of the directories in your $CDPATH 
-- if this shell variable is defined (no pathname needed).
01ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



000137echo $HOME

0032if [ -d $F00 ] ;then exit; fi ; if [ "$CDPATH" ] ; then for j in `echo $CDPATH | /usr/bin/awk -F: '{ for (i = 1; i <= NF; i++) print $i }'`; do if [ -d ${j}/$F00 ]; then exit; fi; done; fi; echo "DIRECTORY DOES NOT EXIST"


001010echo $HOME
011010




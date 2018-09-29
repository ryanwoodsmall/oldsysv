#ident	"@(#)forms.files:echo.fs	1.4"
00000echo
00702060030ARGUMENT(S):
01704060030ADDITIONS TO THE COMMAND LINE:

00012
01022


"echo" prints strings on your terminal.  One use
of "echo" is to find out the current values of your
environment variables.  For example, to find out your
current login directory, you could enter "echo $HOME".

00Enter the arguments you want printed.  Normal text will be printed
without modification.  Any defined shell variable (e.g. $HOME) will
be replaced by its current value (e.g. echo $HOME).  
If you do not want a shell variable expanded, use single quotes.  
If you enter multiple arguments here, each will be printed in 
the order entered.
01ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field, 
you should use it with caution.






001110
011110




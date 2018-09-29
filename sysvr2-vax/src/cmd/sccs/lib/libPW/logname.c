static char Sccsid[]="@(#)logname.c	6.1";
char *
logname()
{
	return((char *)getenv("LOGNAME"));
}

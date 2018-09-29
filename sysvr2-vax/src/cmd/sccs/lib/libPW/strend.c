static char Sccsid[]="@(#)strend.c	6.1";
char *strend(p)
register char *p;
{
	while (*p++)
		;
	return(--p);
}

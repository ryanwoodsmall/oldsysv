static char SCCSID[]="@(#)ctos.c	1.1";
char *ctos(c) /* convert char to string */
char c;
{
	static char s[2] = { '\0','\0' };
	s[0] = c;
	return(s);
}

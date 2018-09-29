/*	@(#)console.c	1.1	*/
#define	KL	0177560
struct	{	char lobyte, hibyte;};
struct	{	int ks, kb, ps, pb;};
getchar() {
	register rc;

	KL->ks++;
	while(KL->ks.lobyte>=0);
	rc = KL->kb;
	rc =& 0177;
	if('A'<=rc && rc<='Z')
		rc =+ 'a'-'A';
	if(rc=='\r')
		rc = '\n';
	putchar(rc);
	return(rc);
}

putchar(c)
{
	register rc;

	rc = c;
	while(KL->ps.lobyte>=0);
	if(rc==0)
		return;
	if(rc=='\n') {
		putchar('\r');
		putchar('\n'|0200);
		putchar(0);
	} else
		KL->pb = rc;
	return(rc);
}


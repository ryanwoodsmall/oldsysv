/*	@(#)signal_.c	1.2	*/
/*	3.0 SID #	1.2	*/
signal_(sigp, procp)
int *sigp, (**procp)();
{
return( signal(*sigp, *procp) );
}

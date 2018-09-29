/*	@(#)signal_.c	1.3	*/

signal_(sigp, procp,L)
long L;
int *sigp, (*procp)();
{
return( signal(*sigp, *procp) );
}

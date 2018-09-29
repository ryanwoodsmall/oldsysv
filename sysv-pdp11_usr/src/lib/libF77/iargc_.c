/*	@(#)iargc_.c	1.2	*/
long int iargc_()
{
extern int xargc;
return ( xargc - 1 );
}

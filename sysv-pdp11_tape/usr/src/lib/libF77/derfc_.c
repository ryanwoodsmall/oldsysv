/*	@(#)derfc_.c	1.2	*/
double derfc_(x)
double *x;
{
double erfc();

return( erfc(*x) );
}

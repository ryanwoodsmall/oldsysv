/*	@(#)d_cnjg.c	1.2	*/
#include "complex"

d_cnjg(r, z)
dcomplex *r, *z;
{
r->dreal = z->dreal;
r->dimag = - z->dimag;
}

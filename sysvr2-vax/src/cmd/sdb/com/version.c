 	/* @(#) version.c: 1.11 6/20/84 */

version()
{
#if u3b
	error("AT&T Technologies 3B-20 sdb System V Release 2.0");
#else
#if u3b5
	error("AT&T Technologies 3B-5 sdb System V Release 2.0");
#else
#if vax
	error("VAX-11/780 sdb System V Release 2.0");
#else
#if u3b2
	error("AT&T Technologies 3B-2 sdb System V Release 2.0");
#endif
#endif
#endif
#endif
}

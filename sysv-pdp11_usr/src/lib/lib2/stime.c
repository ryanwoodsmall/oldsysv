/*	@(#)stime.c	1.1	*/

stime(tloc)
long *tloc; {
	extern long _time;

	_time = *tloc;
}

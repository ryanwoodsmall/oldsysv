/*	@(#)time.c	1.1	*/

long	_time;

long
time(tloc)
long *tloc; {

	if (tloc)
		*tloc = _time;
	return (_time);
}

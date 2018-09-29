/*	@(#)tell.c	1.1	*/

# include <stand.h>

off_t
tell(fildes)
{
	extern off_t lseek();

	return (lseek(fildes, (off_t) 0, 1));
}

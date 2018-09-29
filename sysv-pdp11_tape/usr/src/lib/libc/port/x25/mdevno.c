/*	@(#)mdevno.c	1.2	*/
#include <sys/types.h>
#include <sys/stat.h>
#include <x25lib.h>

int
mdevno(file)
char *file;
{
	struct stat *bp;
	struct stat buf;

	bp = &buf;
	if(stat(file,bp)){
		return(MDERR1);
	}
	if ( (bp->st_mode & S_IFMT) != S_IFCHR) {
		return(MDERR2);
	}
	return(bp->st_rdev&0377);
}

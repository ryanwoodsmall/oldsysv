/*	@(#)defs.c	1.2	*/

#include	"defs.h"

struct layer 	*layers[MAX_LAYERS];

int max_index = 0;
int chan = 0;
int fildes[2];
	
char cntlf[]	= "/dev/sxt/000";
int	cntl_chan_fd;
	
struct utmp	  *u_entry;
struct termio ttysave;

int uid;
int gid;

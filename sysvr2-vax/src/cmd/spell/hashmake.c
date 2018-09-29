/*	@(#)hashmake.c	1.1	*/
#include "hash.h"

main()
{
	char word[30];
	long h;
	hashinit();
	while(gets(word)) {
		printf("%.*lo\n",(HASHWIDTH+2)/3,hash(word));
	}
	return(0);
}

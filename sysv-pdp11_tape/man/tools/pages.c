#include <stdio.h>

#define T_INIT	0100
#define T_STOP	0111


main(argc, argv)
char	**argv;
{

	register int	npages = 0;	/* number of pages */
	register long	charin = 0;	/* number of input character */
	register int	c;		/* the character gotten */

	while((c=getchar()) != EOF) {
		charin++;
		c &= 0377;
		if(c != T_INIT)
			continue;
		else {
			c=getchar();
			c &= 0377;
			if(c == T_STOP) {
				npages++;
				charin++;
			}
		}
	}
	if(charin<5) {
		fprintf(stderr, "%s: no input\n", argv[0]);
		exit(1);
	}
	printf("%d\n", npages);
}
/*	@(#)pages.c	5.2 of 5/19/82	*/

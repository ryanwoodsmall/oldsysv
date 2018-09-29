/*	vpmsave.c 1.6 of 8/13/82
	@(#)vpmsave.c	1.6
 */


#include <stdio.h>
#include "sys/types.h"
#include "sys/csi.h"
#include "sys/vpmt.h"

static char Sccsid[] = "@(#)vpmsave.c	1.6";
char ev[512];
main(argc, argv)
char *argv[];
{
	int fd, n, k, seqno, chbits;

	if (argc != 3) {
		fprintf(stderr, "Incorrect number of arguments\n");
		fprintf(stderr, "Usage: vpmsave mask device\n");
		exit(1);
	}
	if ((fd = open(argv[2], 2)) < 0) {
		perror("vpmsave open:");
		exit(2);
	}
	setbuf(stdout, NULL);
	sscanf(argv[1], "%6o", &chbits);
	if ((k = ioctl(fd, VPMSETC, chbits)) < 0) {
		perror("vpmsave ioctl:");
		exit(3);
	}
	seqno = 1;
	for (;;) {
		if ((n = read(fd, ev, 512)) < 0) {
			perror("vpmsave read:");
			exit(4);
		}
		if (write(1, ev, n) < 0) {
			perror("vpmsave write:");
			exit(5);
		}
	}
}

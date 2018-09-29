/*	vpmsave.c 1.3 of 2/10/82
	@(#)vpmsave.c	1.3
 */


#include <stdio.h>
#include "sys/types.h"
#include "sys/csi.h"
#include "sys/vpmt.h"

static char Sccsid[] = "@(#)vpmsave.c	1.3";
char ev[512];
main(argc, argv)
char *argv[];
{
	int fd, n, k, seqno, chbits;

	if (argc != 3) {
		fprintf(stderr, "Arg count\n");
		exit();
	}
	if ((fd = open(argv[2], 2)) < 0) {
		perror();
		exit();
	}
	setbuf(stdout, NULL);
	sscanf(argv[1], "%6o", &chbits);
	if ((k = ioctl(fd, VPMSETC, chbits)) < 0) {
		perror();
		exit();
	}
	seqno = 1;
	for (;;) {
		if ((n = read(fd, ev, 512)) < 0) {
			perror("read");
			exit();
		}
		if (write(1, ev, n) < 0) {
			perror("write");
			exit();
		}
	}
}

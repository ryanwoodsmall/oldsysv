/* @(#)vrmhead.c	1.2 */
# include "a.out.h"

#define	VAXWRMAGIC	0570
#define	VAXROMAGIC	0575
#define	AOUT1MAGIC	0407
#define	AOUT2MAGIC	0410
#define	AOUT3MAGIC	0413

struct wd {
	short hiword;
	short loword;
};

fixl (p)
register long *p; 
{
	register short t;

	t = ((struct wd *) p)->hiword;
	((struct wd *) p)->hiword = ((struct wd *) p)->loword;
	((struct wd *) p)->loword = t;
}

main (argc, argv)
char *argv[]; 
{
	int headsize;
	int ifd, ofd, nbyt, ttt;
	register char *infile, *oufile, *ap;
	char *fbuf, zero;

	struct sgshdr {
		struct	filehdr  filhdr;
		struct	aouthdr  naout;
		struct	scnhdr  scnhdr[3];
	} sgshdr;

	struct {
		short magic, stamp;
		long tlen, dlen;
		long blen, slen, eadr, trl, drl;
	} 
	head;


	char buf[512];
	char *sbrk ();

	zero = '\0';
	if (argc != 3) {
usage:
		printf ("usage: vrmhead input output\n");
		exit (1);
	}

	infile = argv[1];
	oufile = argv[2];

	if ((ifd = open (infile, 0)) < 0) {
		ap = infile;
oerr:
		printf ("%s: cannot open\n", ap);
		exit (1);
	}

	if ((ofd = creat (oufile, 0777)) < 0) {
		ap = oufile;
		goto oerr;
	}

	if (read (ifd, &head, sizeof head) != (sizeof head)) {
		printf ("%s ",argv [0]);
		printf (": read error\n");
		exit (1);
	} else {
		switch (head.magic) {
		case AOUT1MAGIC:
		case AOUT2MAGIC:
		case AOUT3MAGIC:
			headsize = sizeof head;
			break;

		case VAXWRMAGIC:
		case VAXROMAGIC:
			lseek(ifd,  0, 0);
			read(ifd, (char *)&sgshdr, sizeof(struct sgshdr));
			headsize = sizeof(struct sgshdr);
			head.magic = sgshdr.naout.magic;
			head.tlen = sgshdr.naout.tsize;
			head.dlen = sgshdr.naout.dsize;
			break;
		default:
			printf("Unknown a.out type: %o\n", head.magic);
			exit(1);
		}
	}
#ifndef vax
	fixl (&head.tlen); 
	fixl (&head.dlen);
#endif
	printf("seeking to headsize=%x\n", headsize);
	if (lseek (ifd, headsize, 0) < 0) {
		printf ("input seek error\n");
unlnk:
		unlink (oufile);
		exit (1);
	}

	fbuf = sbrk (4096);
	printf("head.tlen=%x head.dlen=%x\n", head.tlen, head.dlen);
	while (head.tlen > 0) {
		if (head.tlen > 4096)
			nbyt = 4096; 
		else
			nbyt = head.tlen;
		read (ifd, fbuf, nbyt);
		write (ofd, fbuf, nbyt);
		head.tlen -= nbyt;
	}
	while (nbyt & 0777) {
		write (ofd, &zero, 1); 
		++nbyt;
	}
	while (head.dlen > 0) {
		if (head.dlen > 4096)
			nbyt = 4096; 
		else
			nbyt = head.dlen;
		read (ifd, fbuf, nbyt);
		write (ofd, fbuf, nbyt);
		head.dlen -= nbyt;
	}
	return (0);
}

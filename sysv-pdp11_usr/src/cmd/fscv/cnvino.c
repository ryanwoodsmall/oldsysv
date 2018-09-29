/* @(#)cnvino.c	1.1 */
cnvino(tovax,fdo,dblk)
int tovax,fdo;
unsigned short dblk;
{

#include <stdio.h>

#include <sys/param.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/ino.h>
#include <sys/inode.h>

	int in,out,j,k;
	long i,indadr[3],fsize,size;

	/* Build an array of inodes */
	struct dinode d[INOPB];


	/* Set a variable depending whether or not program is running on a vax */
#ifdef vax
#define ONVAX 1
#else
#define ONVAX 0
#endif

	/* Read in blocks of inodes */

	for (i = 2; i < dblk; i++) {

		/* Point to next inode */
		lseek(fdo,i*512,0);

		in = read(fdo,d,512);
		if (in <= 0) {
			perror("Error reading inode");
			return(-1);
		}

		/* Do conversions for each inode */
		for (j=0; j < INOPB; j++) {

			/* Convert the proper type of inode */
			switch(d[j].di_mode&IFMT) {
			case 0:
				break;

			case IFDIR:
			case IFREG:
			case IFCHR:
			case IFBLK:
				{

					/*
										 * Set up the list of the three indirect blocks.  This is done
										 * after the inode conversion if program is running on the
										 * machine to which file system is being converted.  (See indino.c)
										 */
					if(ONVAX != tovax) {
						l3tol(indadr,&d[j].di_addr[30],3);
						fsize = d[j].di_size;
					}
					swap(&d[j].di_size);
					tovax ?
					(inov(&d[j].di_addr[0])) :
					(inop(&d[j].di_addr[0])) ;
					if(ONVAX == tovax) {
						l3tol(indadr,&d[j].di_addr[30],3);
						fsize = d[j].di_size;
					}
					/* Convert the indirect blocks */
					for(k = 0; k < 3; k++) {
						/* Pass the size of the file so far */
						size = 5120;

						indino(tovax,fdo,fsize,&size,k,indadr[k]);
					}

				}
				/*
				 * Special handling when implemented
				 * case IFMPC:
				 * case IFMPB:
				 * case IFIFO:
				 */
				swap(&d[j].di_atime);
				swap(&d[j].di_mtime);
				swap(&d[j].di_ctime);
			}
		}
		/* Write inode block back out */
		lseek(fdo,i*512,0);
		out = write(fdo,d,512);
		if (in != out) {
			printf(stderr,"cnvino: error writing output file\n");
			return(1);
		}
	}
	return(0);
}

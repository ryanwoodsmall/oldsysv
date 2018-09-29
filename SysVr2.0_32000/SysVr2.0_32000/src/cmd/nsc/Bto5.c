#include <sys/param.h>
#include <sys/types.h>
#include <sys/filsys.h>
#include "sys/Bfilsys.h"

struct filsys f;
struct Bfilsys b;

main(argc, argv)
char **argv;
{
	int i, j, k;

	if (argc != 2) {
		printf("Usage: %s dev\n", argv[0]);
		exit(1);
	}
	close(0);
	i = open(argv[1], 2);
	if (i != 0) {
		printf("Cant open %s\n", argv[1]);
		exit(2);
	}
	lseek(0, 1024, 0);
	read(0, &b, sizeof(b));
	printf("isize %d fsize %d\n", b.s_isize, b.s_fsize);
	f.s_isize = b.s_isize;
	f.s_fsize = b.s_fsize;
	f.s_nfree = b.s_nfree;
	f.s_ninode = b.s_ninode;
	f.s_time = b.s_time;
	f.s_tfree = b.s_tfree;
	f.s_tinode = b.s_tinode;
	j = b.s_nfree;
	if (j > NICFREE) {
		j = NICFREE;
		f.s_nfree = j;
	}
	for (i=0; i<j; i++)
		f.s_free[i] = b.s_free[i];
	for (i=0; i<b.s_ninode; i++)
		f.s_inode[i] = b.s_inode[i];
	f.s_magic = FsMAGIC;
	f.s_type = Fs2b;
	lseek(0, 512, 0);
	write(0, &f, 512);
}

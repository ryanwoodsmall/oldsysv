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
	lseek(0, 512, 0);
	read(0, &f, sizeof(f));
	printf("isize %d fsize %d\n", f.s_isize, f.s_fsize);
	printf("magic %x type %d\n", f.s_magic, f.s_type);
	if (f.s_magic != 0xfd187e20 || f.s_type != Fs2b) {
		printf("bad 5.0 FS\n");
		exit(1);
	}
	b.s_isize = f.s_isize;
	b.s_fsize = f.s_fsize;
	b.s_nfree = f.s_nfree;
	b.s_ninode = f.s_ninode;
	b.s_time = f.s_time;
	b.s_tfree = f.s_tfree;
	b.s_tinode = f.s_tinode;
	for (i=0; i<f.s_nfree; i++)
		b.s_free[i] = f.s_free[i];
	for (i=0; i<f.s_ninode; i++)
		b.s_inode[i] = f.s_inode[i];
	lseek(0, 1024, 0);
	write(0, &b, sizeof(b));
}

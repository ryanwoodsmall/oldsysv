#include <sys/types.h>
#include <sys/mtio.h>

struct mtop mtop;


struct cmds {
	char name[16];
	int cmd;
} coms[] = {
	{ "rewind", MTREW },
	{ "erase", MTERASE },
	{ "retension", MTRETEN },
	{ "display", MTDISPL },
	{ "weof", MTWEOF },
	{ "fsf", MTFSF },
	{ "fsr", MTFSR },
	0
};

main(argc, argv)
char **argv;
{
	int i, j, k;
	register struct cmds *p;
	extern char errno;

	i = open("/dev/rmt/0mn", 2);
	if (i < 0) {
		printf("Cant open tape\n");
		exit(1);
	}
	if (argc < 2) {
		printf("Usage: %s command [count]\n", argv[0]);
		exit(2);
	}
	for (p=coms; p->name[0] != 0; p++) {
		if (strcmp(p->name, argv[1]) == 0)
			goto found;
	}
	printf("Options:\n");
	for (p=coms; p->name[0] != 0; p++) {
		printf("%s\n", p->name);
	}
	exit(3);
found:
	mtop.mt_op = p->cmd;
	if (argc > 2)
		mtop.mt_count = atoi(argv[2]);
	else
		mtop.mt_count = 1;
	j = ioctl(i, MTIOCTOP, &mtop);
	if (j < 0 ) {
		printf("errno %d\n", errno);
		perror("bad ioctl");
	}
}

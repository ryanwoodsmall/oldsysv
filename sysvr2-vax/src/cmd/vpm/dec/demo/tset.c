/*	tset.c 1.2 of 9/24/81
 */

static char Sccsid[] = "@(#)tset.c	1.2";

#include <stdio.h>

char	msg[] = "sdflkjavp8wtfmn v;uzxv9wrjhd vpizvpf q98bdfkjhfb v98f:2rwhf sdf s8";
int count;
char	buff[512];

main(argc,argv)
char	*argv[];
{
	int fd;

	if(argc != 2){
		fprintf(stderr,"Arg count\n");
		exit(1);
	}
	printf("open\n");
	if((fd = open(argv[1],2)) < 0){
		fprintf(stderr,"Can't open %s\n",argv[1]);
		perror("main");
		myexit(13);
	}
	printf("write\n");
	count = write(fd,msg,sizeof msg);
	printf("count=%d\n",count);
	if (count != sizeof msg)
		myexit(14);
	count = read(fd,buff,512);
	printf("read count = %d\n",count);
	printf("String = %s\n",buff);
	if (count != sizeof msg)
		myexit(15);
	if (strcmp(msg,buff) != 0){
		printf("String = %s\n",msg);
		myexit(16);
	}
	printf("It worked !!!!!!");
	myexit(0);
}
myexit(c)int c;
{
	int i;
	printf("exit code = %d\n",c);
	if(c!=0)
		for(i=0;i<count;i++)
			printf("%c %o\n",buff[i],buff[i]);
	exit(c);
}

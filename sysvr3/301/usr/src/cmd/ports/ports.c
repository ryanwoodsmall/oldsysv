/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ports:ports.c	1.7"
/*
 *	Copyright 1984 AT&T
 */

#include "stdio.h"
#include "fcntl.h"
#include "sys/sysmacros.h"
#include "sys/edt.h"
#include "sys/sys3b.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/dir.h"

#define PORT_CODE	3
#define MAXSLOTS	13
#define MAXSUBDEV	30

extern int errno;

short index[MAXSLOTS] ={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
short ttycor[MAXSLOTS] ={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
short slot[MAXSLOTS] ={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

struct table {
	struct stat statb;
	char pathname[25];
	short istty;
} table[MAXSLOTS][MAXSUBDEV];

struct table temp;
struct direct dir;

struct edtsize {
	short esize;
	short ssize;
} edtsize;

struct edt *edtptr;


main()
{
	char buffer[150][100];
	char cmd[100];
	char device[25];
	FILE *inittab;
	FILE *newfile;
	short isttyconf = 1;
	int cmask = 0;
	int fd;
	short maj;
	char *ptr;
	int edt_size;
	char *path = "/dev";
	int i, j, n = 0;

     	if(sys3b(S3BEDT,&edtsize,sizeof(struct edtsize)) != 0)
		err("Ports: Sys3b call to get edt table failed.\nCall your local service representative.\n");
    
     	edt_size = edtsize.esize * sizeof(struct edt) + edtsize.ssize * sizeof(struct subdevice) + sizeof(struct edtsize);

     	if((edtptr = (struct edt *)malloc(edt_size)) == NULL)
		err("Ports: Not enough space to allocate memory.\nPlease call your local service representative.\n");

     	if(sys3b(S3BEDT,edtptr,edt_size) != 0)
		err("Ports: Sys3b call to get edt table failed.\nCall your local service representative.\n");

	edtptr = (struct edt *) ((char *)edtptr + 4);

     	for(i = 0; i < edtsize.esize; ++i)
	{
		if(edtptr->opt_code == PORT_CODE)
			slot[edtptr->opt_slot] = (short)edtptr->opt_slot;

		edtptr++;
	}

/* Read the device directory into buffer */

	i = j = 0;

	if((fd = open("/dev", O_RDONLY)) < 0)
	{
		printf("Ports: Cannot open /dev directory.\n Error %d: See Unix Users Manual - Intro(2)\n", errno);
		exit(errno);
	}

	while(read(fd, &dir, sizeof(struct direct)) > 0)
	{
		if(dir.d_name[0] != '.')
		{
			sprintf(temp.pathname, "%s/%s", path, dir.d_name);

			if(stat(temp.pathname, &temp.statb) < 0)
				continue;

			if(((temp.statb.st_mode&S_IFMT) == S_IFCHR) && (dir.d_ino != 0))
			{
				maj = major(temp.statb.st_rdev);

				if(maj > 0 && maj < MAXSLOTS)
				{
					table[maj][index[maj]] = temp;
					if(maj <= 9)
					{
						if((cmp(dir.d_name, "tty", 3) == 0) && ((dir.d_name[3] - '0') == maj))
						{
							ttycor[maj] = 1;
							table[maj][index[maj]].istty = 1;
						}
					}
					else
					{
						if(cmp(dir.d_name, "tty", 3) == 0 && (((dir.d_name[3] - '0')*10) + (dir.d_name[4] - '0')) == maj)
						{
							ttycor[maj] = 1;
							table[maj][index[maj]].istty = 1;
						}
					}
					index[maj]++;
				}
			}
		}
	}

	close(fd);
	umask(cmask);
	for(i = 0; i < MAXSLOTS; i++)
	{
		if(slot[i] != 0)
		{
			if(index[i] != 0)
			{
				if(ttycor[i] == 0)
				{
					isttyconf = 0;

					for(j = 0; j < index[i]; j++)
					{
						printf("Warning: %s is being removed.\n", table[i][j].pathname);
						unlink(table[i][j].pathname);
					}
					for(j = 1; j < 6; j++)
					{
						sprintf(device, "%s/tty%d%d", path,i,j);
						mknod(device, 0020622, ((i<<8)|(j-1)));
						if(j == 1)
						{
							sprintf(cmd, "/etc/pump %s /lib/pump/ports", device);
							system(cmd);
						}
					}
				}
				else
				{
					sprintf(cmd, "/etc/pump %s/tty%d1 /lib/pump/ports", path, i);
					system(cmd);

				}
			}
			else
			{
				isttyconf = 0;
				for(j = 1; j < 6; j++)
				{
					sprintf(device, "%s/tty%d%d", path,i,j);
					mknod(device, 0020622, ((i<<8)|(j-1)));

					if(j == 1)
					{
						sprintf(cmd, "/etc/pump %s /lib/pump/ports", device);
						system(cmd);
					}
				}
			}
		}
		else
		{

			if(ttycor[i] != 0)
			{
				isttyconf = 0;
				for(j = 0; j < index[i]; j++)
				{
					if(table[i][j].istty == 1)
					{
						unlink(table[i][j].pathname);
					}
					else
					{
						printf("Warning: %s is being removed.\n", table[i][j].pathname);
						unlink(table[i][j].pathname);
					}
				}
			}
		}
	}

	if(isttyconf == 1)
	{
		edtptr = (struct edt *)(((char *)(edtptr - edtsize.esize)) - (char *)4);
		free(edtptr);
		exit(0);
	}
	stat("/etc/inittab", &temp.statb);

	if((inittab = fopen("/etc/inittab", "r")) == NULL)
		err("/etc/inittab cannot be opened for reading and writing.  Please call your local service representative.\n");

	if(creat("/etc/newfile", 0000666) < 0)
	{
		printf("Ports: Error %d - Wasn't able to create a temporary file\n", errno);
		exit(errno);
	}

	if((newfile = fopen("/etc/newfile", "r+w+a+")) == NULL)
		err("/etc/newfile cannot be opened for reading and writing.  Please call your local service representative.\n");

	i = 0;
	fflush(inittab);
	fflush(newfile);

	while(fgets(ptr = buffer[i], 512, inittab) != NULL)
	{
		if(buffer[i][0] > '0' && buffer[i][0] <= '9')
		{
			if(buffer[i][1] >= '0' && buffer[i][1] <= '9')
			{
				while(*ptr++ != '\0')
				{
					if(cmp("tty", &ptr[0], 3) == 0)
					{
						if(buffer[i][2] == ':')
						{
							if(slot[buffer[i][0] - '0'] == 0)
								buffer[i][0] = 0x7f;
						}
						else
						{
							if(slot[(((buffer[i][0] - '0')*10)+(buffer[i][1] - '0'))] == 0)
								buffer[i][0] = 0x7f;
						}
					}
				}
			}
		}
		i++;
	}

	for(j = 0; j < MAXSLOTS; j++)
	{
		for(n = 1; n < 6; n++)
		{
			if(slot[j] != 0 && ttycor[j] == 0)
				sprintf(buffer[i++], "%d%d:234:off:/etc/getty tty%d%d 9600\n", j, n, j, n);
			else
				break;
		}
	}

	j = 0;

	while(j < i)
	{
		if(buffer[j][0] != 0x7f)
			fputs(buffer[j], newfile);

		j++;
	}

	edtptr = (struct edt *)(((char *)(edtptr - edtsize.esize)) - (char *)4);
	free(edtptr);
	fclose(inittab);
	fclose(newfile);
	unlink("/etc/inittab");
	link("/etc/newfile", "/etc/inittab");
	unlink("/etc/newfile");
	chown("/etc/inittab", temp.statb.st_uid, temp.statb.st_gid);
	chmod("/etc/inittab", temp.statb.st_mode&0000777);
}

err(s)
char *s;
{
	printf("%s", s);
	exit(1);
}

cmp(s1, s2, cnt)
char *s1, *s2;
int cnt;
{
	int n = 0;

	while(n++ < cnt)
		if(*s1++ != *s2++)
			return(-1);

	return(0);
}

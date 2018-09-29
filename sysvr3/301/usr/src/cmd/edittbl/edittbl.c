/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)edittbl:edittbl.c	1.6"
#include <stdio.h>

#ifdef m32

#include <sys/sys3b.h>
#include <sys/edt.h>

#define NEDTP(X) (nedtp +X)

	/*structure for boards to be renamed */
struct	{
	int slot;		/* slot number for board */
	char swname[E_NAMLEN];	/* software name of device */
	char hwname[E_NAMLEN];	/* hardware name of device */
	} rename[MAX_IO+1];

#endif

#define ON 1
#define OFF 0

#define E_NAMLEN 10

/*******************************************************************/
/*	edt_def.h - definitions associated with the edt		   */
/*		must follow diagnostic.h			   */
/*******************************************************************/


#define D_MAX 16	/* limit for lookup table device entries */

#define SD_MAX 132	/* limit for lookup table subdevice entries */

struct dev_code {
	unsigned opt_code:16;	/* code for device in option slot */
	unsigned rq_size:8;	/* size of request queue for device */
	unsigned cq_size:8;	/* size of completion queue */
	unsigned boot_dev:1;	/* set =1 if possible boot device */
	unsigned word_size:1;	/* zero = 8 bit */
				/* one = 16 bit */
	unsigned brd_size:1;	/* zero = single width */
				/* one = double width */
	unsigned smrt_brd:1;	/* zero = dumb board */
				/* one = intelligent board */
	unsigned cons_cap:1;	/* set =1 if can support console */
	unsigned cons_file:1;	/* set =1 if cons pump file needed */
	unsigned pad:26;
	unsigned char dev_name[E_NAMLEN];	/* ASCII name for device */
	unsigned char diag_file[E_NAMLEN];	/* diagnostic file name */
};


typedef struct {
	unsigned long num_dev;	/* number of entries in device lookup table */
	struct dev_code  dev_code[D_MAX];	/* maximum lookup entries =D=MAX */
}DEV_TAB;





struct sbdev_code {
	unsigned short opt_code;	/* code for subdevices in option slot */
	unsigned char name[E_NAMLEN];	/* subdevice name	*/
};



typedef struct {
	unsigned char num_sbdev;	/* entries in subdev lookup table */
	unsigned char pad[3];   	/* pad so edittbl compiles in the */
					/*   cross-environment            */
	struct sbdev_code sbdev_code[SD_MAX];	/* lookup table max=SD_MAX */
}SBDEV_TAB;





/* define device table structure */
DEV_TAB p_devt;		

/* define subdevice table structure */

SBDEV_TAB p_sbdevt; 

#define TABLSIZE	sizeof(DEV_TAB) + sizeof(SBDEV_TAB)

FILE *fopen(), *fp;

char filename[8*E_NAMLEN];
char install,remove,list,dev,sbdev,gen,terse;
int opt_code,rq_size,cq_size,boot_dev,word_size,brd_size,smrt_brd,cons_cap,cons_file;
char dev_name[E_NAMLEN],sdv_name[E_NAMLEN];

main(argc,argv)

int argc;
char *argv[];
{
	int i,j;
	char openmode[E_NAMLEN];
	DEV_TAB *P_DEVT;
	SBDEV_TAB *P_SBDEVT;


	if (argc == 1)
		{
		help();
		exit(0);
		}

	for (i = 1; i < argc; i++)
		{
		if ( argv[i][0] != '-')
			strcpy(filename,argv[i]);

		else if ( !strcmp(argv[i], "-i" ) )
			install = ON;

		else if ( !strcmp(argv[i], "-r") )
			remove = ON;

		else if ( !strcmp(argv[i], "-l") )
			list = ON;

		else if ( !strcmp(argv[i], "-d") )
			dev = ON;

		else if ( !strcmp(argv[i], "-s") )
			sbdev = ON;

		else if ( !strcmp(argv[i], "-g") )
			gen = ON;

		else if ( !strcmp(argv[i], "-t") )
			terse = ON;

		else	{
			printf ("invalid argument\n");
			exit(1);
			}
		}

	if (terse != ON)
			printf("utility program for edt_data\n\n");

	if ( !strcmp(filename,"\0"))
		strcpy(filename,"edt_data");

	if ((install == ON && remove == ON) || (gen == ON && remove == ON))
		{
		printf ("cannot install/generate and remove\n");
		exit(1);
		}

	if ((install == ON || remove == ON) && dev == ON && sbdev == ON)
		{
		printf ("cannot install/remove device and subdevice tables\n");
		exit(1);
		}


	if (dev != ON && sbdev != ON)
		{
		printf("neither device nor subdevice table selected\n");
		exit(1);
		}

	if (install != ON && remove != ON && gen != ON && list != ON)
		{
		printf("no function specified for table(s)\n");
		exit();
		}


	P_DEVT = &p_devt;
	P_SBDEVT = &p_sbdevt;

	/* open file */

	if ((fp = fopen(filename, install|remove|gen ? "r+" : "r" )) == NULL)
		{
		if (gen != ON)
			{
			/* print error message if file missing when generate not specified */
			perror(filename);
			exit(1);
			}

		if ((fp = fopen(filename,"w+")) == NULL)
			{
			printf("open failed for %s\n",filename);
			exit(1);
			}
		}

	else
		{
		fread(P_DEVT,sizeof(DEV_TAB),1,fp);
		fread(P_SBDEVT,sizeof(SBDEV_TAB),1,fp);
		fseek(fp,0L,0);
		}

	if (gen == ON && dev == ON)
		{
		gendev();
		}

	if (gen == ON && sbdev == ON)
		{
		gensbdev();
		}

	if (remove == ON && dev == ON)
		{
		if (!remdev())
			exit(1);
		}

	if (remove == ON && sbdev == ON)
		{
		if (!remsbdev())
			exit(1);
		}

	if (install == ON && dev == ON)
		{
		if (!instdev())
			exit(1);
		}

	if (install == ON && sbdev == ON)
		{
		if (!instsbdev())
			exit(1);
		}

	if (list == ON && dev == ON)
		{
		listdev();
		}

	if (list == ON && sbdev == ON)
		{
		listsbdev();
		}

	if (install == ON || remove == ON || gen == ON)
		{
		fwrite(P_DEVT,sizeof(DEV_TAB),1,fp);
		fwrite(P_SBDEVT,sizeof(SBDEV_TAB),1,fp);
		}

	fclose(fp);

	exit(0);
}


instdev()
{
int i,s;

DEV_TAB *P_DEVT;
SBDEV_TAB *P_SBDEVT;

P_DEVT = &p_devt;
P_SBDEVT = &p_sbdevt;

if (P_DEVT->num_dev >= D_MAX)
	{
	printf("Device table is full\n\n");
	return(0);
	}

/* clear input variables before read */
opt_code = rq_size = cq_size = boot_dev = word_size = brd_size = smrt_brd = cons_cap = cons_file = 0;
dev_name[0] = '\0';

if (terse != ON)
	printf("Enter device data\n");

/* assign values for devices */

while ((s = dv_in()) != EOF && s != 0)
	{
	/* truncate input strings that exceed defined length */
	dev_name[E_NAMLEN-1] = '\0';

	for (i = 0; i < P_DEVT->num_dev; i++)
		{
		if (P_DEVT->dev_code[i].opt_code == opt_code)
			break;

		if (i ==  P_DEVT->num_dev -1)
			{
			(P_DEVT->dev_code[P_DEVT->num_dev]).opt_code = opt_code;
			(P_DEVT->dev_code[P_DEVT->num_dev]).rq_size = rq_size;
			(P_DEVT->dev_code[P_DEVT->num_dev]).cq_size = cq_size;
			(P_DEVT->dev_code[P_DEVT->num_dev]).boot_dev = boot_dev;
			(P_DEVT->dev_code[P_DEVT->num_dev]).word_size = word_size;
			(P_DEVT->dev_code[P_DEVT->num_dev]).brd_size = brd_size;
			(P_DEVT->dev_code[P_DEVT->num_dev]).smrt_brd = smrt_brd;
			(P_DEVT->dev_code[P_DEVT->num_dev]).cons_cap = cons_cap;
			(P_DEVT->dev_code[P_DEVT->num_dev]).cons_file = cons_file;

			strcpy((P_DEVT->dev_code[P_DEVT->num_dev]).dev_name,dev_name);
			P_DEVT->num_dev++;
			}
		}
	/* clear input variables before next read */
	opt_code = rq_size = cq_size = boot_dev = word_size = brd_size = smrt_brd = cons_cap = cons_file = 0;
	dev_name[0] = '\0';
	}
return ((s == 0) || (s == EOF) ? ON : OFF);
}



instsbdev()
{
int i, s;

DEV_TAB *P_DEVT;
SBDEV_TAB *P_SBDEVT;

P_DEVT = &p_devt;
P_SBDEVT = &p_sbdevt;

if (P_SBDEVT->num_sbdev >= SD_MAX)
	{
	printf("Subdevice table is full\n\n");
	return(0);
	}

if (terse != ON)
	printf("Enter subdevice data\n");

/* assign values for subdevices */

while ((s = sdv_in ()) != EOF && s != 0)
	{
	/* truncate input strings that exceed defined length */
	sdv_name[E_NAMLEN-1] = '\0';
	dev_name[E_NAMLEN-1] = '\0';

	for (i = 0; i < P_SBDEVT->num_sbdev; i++)
		{
		if (P_SBDEVT->sbdev_code[i].opt_code == opt_code)
			break;

		if (i ==  P_SBDEVT->num_sbdev -1)
			{
			P_SBDEVT->sbdev_code[P_SBDEVT->num_sbdev].opt_code = opt_code;
			strcpy((P_SBDEVT->sbdev_code[P_SBDEVT->num_sbdev]).name, sdv_name);
			P_SBDEVT->num_sbdev++;
			}
		}
	}

return ((s == 0) || (s == EOF) ? ON : OFF);
}

listdev()
{
int i;
DEV_TAB *P_DEVT;
SBDEV_TAB *P_SBDEVT;


P_DEVT = &p_devt;
P_SBDEVT = &p_sbdevt;

/* list assigned values */

printf("num_dev: 0x%x\n",P_DEVT->num_dev);


for (i = 0; i < P_DEVT->num_dev; i++)
	{
	printf("\nID_code: 0x%04x\t", (P_DEVT->dev_code[i]).opt_code);
	printf("dev_name: %9s\t",(P_DEVT->dev_code[i]).dev_name);
	printf("rq_size: 0x%02x\t", (P_DEVT->dev_code[i]).rq_size);
	printf("cq_size: 0x%02x\n", (P_DEVT->dev_code[i]).cq_size);
	printf("boot_dev: %x\t", (P_DEVT->dev_code[i]).boot_dev);
	printf("word_size: %x\t", (P_DEVT->dev_code[i]).word_size);
	printf("brd_size: %x\t", (P_DEVT->dev_code[i]).brd_size);
	printf("smrt_brd: %x\t", (P_DEVT->dev_code[i]).smrt_brd);
	printf("cons_cap: %x\t", (P_DEVT->dev_code[i]).cons_cap);
	printf("cons_file: %x\n", (P_DEVT->dev_code[i]).cons_file);
	}
}


listsbdev()
{
int i;
DEV_TAB *P_DEVT;
SBDEV_TAB *P_SBDEVT;


P_DEVT = &p_devt;
P_SBDEVT = &p_sbdevt;
printf("\nnum_sbdev: 0x%x\n",P_SBDEVT->num_sbdev);

for (i = 0; i < P_SBDEVT->num_sbdev; i++)
	{
	printf("\nID_code: 0x%04x\t", (P_SBDEVT->sbdev_code[i]).opt_code);
	printf("subdev_name: %s\n",(P_SBDEVT->sbdev_code[i]).name);
	}
}


help()
{

printf("         edt_data editor\n\n");
printf("Use:\n");
printf("edittbl [-i] [-r] [-l] [-g] [-t] [file] -d | s\n\n");
printf(" Option     Description\n");
printf(" ======     ==========================\n");
printf("   -i       Install data in a table\n");
printf("   -r       Remove data in a table\n");
printf("   -l       List data in a table\n");
printf("   -g       Generate a basic table\n");
printf("   -t       Suppress user prompts\n");
printf("  file      Full pathname for file\n\n");
printf("   -d       Select the device table\n");
printf("   -s       Select the subdevice table\n");
}

remdev()
{
int i,s;
int scan_flag = ON;	/* set scan flag to a success value */
int j,temp_num;

DEV_TAB *P_DEVT;
SBDEV_TAB *P_SBDEVT;

P_DEVT = &p_devt;
P_SBDEVT = &p_sbdevt;


if (terse != ON)
	printf("Enter device data\n");

while ((s = dv_in ()) != EOF && s != 0)
	{
	temp_num = P_DEVT->num_dev;

	for (i = 0; i < P_DEVT->num_dev; i++)
		{
		if (P_DEVT->dev_code[i].opt_code == opt_code && (scan_flag &= dev_scan()) == ON)
			{
			temp_num--;

			if (i < D_MAX - 1)
				for (j = i; j < temp_num; j++)
					{
					(P_DEVT->dev_code[j]).opt_code =
						(P_DEVT->dev_code[j + 1]).opt_code;

					(P_DEVT->dev_code[j]).rq_size =
						(P_DEVT->dev_code[j + 1]).rq_size;

					(P_DEVT->dev_code[j]).cq_size =
						(P_DEVT->dev_code[j + 1]).cq_size;

					(P_DEVT->dev_code[j]).boot_dev =
						(P_DEVT->dev_code[j + 1]).boot_dev;

					(P_DEVT->dev_code[j]).word_size =
						(P_DEVT->dev_code[j + 1]).word_size;

					(P_DEVT->dev_code[j]).brd_size =
						(P_DEVT->dev_code[j + 1]).brd_size;

					(P_DEVT->dev_code[j]).cons_cap =
						(P_DEVT->dev_code[j + 1]).cons_cap;

					(P_DEVT->dev_code[j]).cons_file =
						(P_DEVT->dev_code[j + 1]).cons_file;

					(P_DEVT->dev_code[j]).smrt_brd =
						(P_DEVT->dev_code[j + 1]).smrt_brd;
					strcpy((P_DEVT->dev_code[j]).dev_name,
						(P_DEVT->dev_code[j + 1]).dev_name);
					}
			else
				{
				(P_DEVT->dev_code[i]).opt_code = 0;
				(P_DEVT->dev_code[i]).rq_size = 0;
				(P_DEVT->dev_code[i]).cq_size = 0;
				(P_DEVT->dev_code[i]).boot_dev = 0;
				(P_DEVT->dev_code[i]).word_size = 0;
				(P_DEVT->dev_code[i]).brd_size = 0;
				(P_DEVT->dev_code[i]).smrt_brd = 0;
				(P_DEVT->dev_code[i]).cons_cap = 0;
				(P_DEVT->dev_code[i]).cons_file = 0;

				strcpy((P_DEVT->dev_code[i]).dev_name,"");
				}
			}
		}
	P_DEVT->num_dev = temp_num;
	}

	return (((s == 0) || (s == EOF)) && (scan_flag == ON) ? ON : OFF);
}

/* routine to scan 3B2 computer's EDT for occurrences of device ID code */

dev_scan()
{
#ifdef m32
/* compiled for use on a 3B2; make EDT search */

int i,j,rtn;	/* utility variables */
int num_dev;	/* count for devices */
int first_call = ON;	/* first call flag for edt_swapp scanning routine */
int count;	/* count of devices of type to be removed */
struct edt *nedtp;	/* pointer to EDT structure */


	/* read # of devices and subdevices in EDT */

	rtn = sys3b(S3BEDT,&i,sizeof(int));

	/* calculate size of EDT (+ a word for device and subdevice #'s) */

	num_dev = i >> 16;	/* device count in upper two bytes */
	j = num_dev * sizeof(struct edt) + (i & 0xffff) * sizeof(struct subdevice) + sizeof(int);

	/* allocate space */

	if ((nedtp = (struct edt *)(malloc(j))) == (struct edt *)0)
		{
		printf("malloc failed\n");
		return (OFF);
		}

	/* copy EDT into buffer */

	rtn = sys3b(S3BEDT,nedtp,j);

	nedtp = (struct edt *) ((char *)nedtp + 4); /* offset for dev - subdev count */

	count = 0;		/* clear device counter for device to be removed */

	/* search for devices to be removed */

	for (i = 0; i < num_dev; i++)
		{
		/*
		 * if there is another device with matching ID code but
		 * different hardware/software name, check lines in
		 * edt_swapp, if it was readable.
		 */
		if (NEDTP(i)->opt_code == opt_code &&
			strcmp(NEDTP(i)->dev_name, dev_name))
			{
			/*
			 * If first mismatch between HW name and EDT device,
			 * scan /dgn/.edt_swapp file.
			 */
			if (first_call == ON)
				{
				first_call = OFF;	/* clear first call flag */
				rtn = get_swapp(); 	/* save entry count */
				}

			/*
			 * assume device entry is not removable if file was not available,
			 * but scan edt_swapp for a matching slot number if it is
			 */
			if (rtn < 0)
				count++;

			else for (j = 0; j < rtn; j++)
				{
				if (NEDTP(i)->opt_slot == rename[j].slot)
					{
					/* the slots match - this board is still in service */
					count++;
					break;
					}
				}
			}
		}

	/*
	 * do not remove device entry if any other device appears
	 */
	if (count > 0)
		return (OFF);
#endif

/*
 * If compiled for a 3B2, the device is not present elsewhere in the EDT.
 * Return an O.K.
 * If compiled for a non-3B2 machine, return a default O.K.
 */

return(ON);
}

#ifdef m32
/* routine to read contents of /dgn/.edt_swapp into memory */

get_swapp()
{
FILE *f_edt;
int slot, i, conv;
char fname[8 * E_NAMLEN];

strcpy(fname, "/dgn/.edt_swapp"); /* assign path name */

/* open device and print warning if it failed */
if ((f_edt = fopen(fname, "r")) == NULL)
	{
	printf("software application file not found\n");
	return (-1);
	}

/*
 * Read each line from file.  Stop silently on a conversion
 * failure, EOF or at the limit of MAX_IO +1 device entries.
 */

for (i = 0; i < (MAX_IO + 1); i++)
	{
	if ((conv = fscanf(f_edt, "%d %s %s",
		&rename[i].slot, rename[i].swname, rename[i].hwname)) == EOF)
		break;

	if (conv != 3)
		break;
	}

fclose(f_edt);

/* report count for lines read */
return(i);
}
#endif

remsbdev()
{
int i,s;
int scan_flag = ON;	/* set scan flag to a success value */
int j,temp_num;

DEV_TAB *P_DEVT;
SBDEV_TAB *P_SBDEVT;

P_DEVT = &p_devt;
P_SBDEVT = &p_sbdevt;


if (terse != ON)
	printf("Enter subdevice data\n");

while ((s = sdv_in ()) != EOF && s != 0)
	{
	temp_num = P_SBDEVT->num_sbdev;

	for (i = 0; i < P_SBDEVT->num_sbdev; i++)
		{
		/* check first for subdev code match, then see whether EDT is free of codes */
		if (P_SBDEVT->sbdev_code[i].opt_code == opt_code && (scan_flag &= sbdev_scan()) == ON)
			{
			temp_num--;

			if (i < SD_MAX - 1)
				for (j = i; j < temp_num; j++)
					{
					(P_SBDEVT->sbdev_code[j]).opt_code =
						(P_SBDEVT->sbdev_code[j + 1]).opt_code;

					strcpy((P_SBDEVT->sbdev_code[j]).name,
						(P_SBDEVT->sbdev_code[j + 1]).name);
					}
			else
				{
				(P_SBDEVT->sbdev_code[i]).opt_code = 0;

				strcpy((P_SBDEVT->sbdev_code[i]).name,"");
				}
			}
		}
	P_SBDEVT->num_sbdev = temp_num;
	}

	return (((s == 0) || (s == EOF)) && (scan_flag == ON) ? ON : OFF);
}

/* routine to scan 3B 2 computer's EDT for occurrences of subdevice ID code */

sbdev_scan()
{
#ifdef m32
/* compiled for use on a 3B2; make EDT search */

int i,j,rtn;	/* utility variables */
int num_dev;	/* count for devices */
int that_count;	/* count of other devices with subdevice to be removed */
struct edt *nedtp;	/* pointer to EDT structure */


	/* read # of devices and subdevices in EDT */

	rtn = sys3b(S3BEDT,&i,sizeof(int));

	/* calculate size of EDT (+ a word for device and subdevice #'s */

	num_dev = i >> 16;	/* device count in upper two bytes */
	j = num_dev * sizeof(struct edt) + (i & 0xffff) * sizeof(struct subdevice) + sizeof(int);

	/* allocate space */

	nedtp = (struct edt *)(malloc(j));

	/* copy EDT into buffer */

	rtn = sys3b(S3BEDT,nedtp,j);

	nedtp = (struct edt *) ((char *)nedtp + 4); /* offset for dev - subdev count */

	that_count = 0;	/* clear subdevice counter for other devices */

	/* search devices for subdevices to be removed */

	for (i = 0; i < num_dev; i++)
		{
		/* calculate virtual addresses for subdevice pointers */

		NEDTP(i)->subdev = (i == 0) ?
		(struct subdevice *) ((char *)nedtp + num_dev * sizeof(struct edt)):
		(struct subdevice *) ((char *)NEDTP(i-1)->subdev +
		NEDTP(i-1)->n_subdev * sizeof(struct subdevice));

		/* scan subdevices for each device */
		for (j = 0; j < NEDTP(i)->n_subdev; j++)
			{
			/* if there is another subdevice, increment a counter */
			if (NEDTP(i)->subdev[j].opt_code == opt_code)
				{
				if (strcmp(NEDTP(i)->dev_name,dev_name))
					/* device is different type than one removed */
					{
					that_count++;
					break;	/*
					 	* Leave subdevice loop.
					 	* One match per device is enough.
					 	*/
					}
				}
			}
		}

	/*
	 * do not remove subdevice entry if any found in another device type
	 */

	if (that_count > 0)
		return (OFF);
#endif

/*
 * If compiled for a 3B2, the subdevice is not present elsewhere in the EDT.
 * Return an O.K.
 * If compiled for a non-3B2 machine, return a default O.K.
 */

return(ON);
}


gendev()
{
	int i;
	DEV_TAB *P_DEVT;
	SBDEV_TAB *P_SBDEVT;

	P_DEVT = &p_devt;
	P_SBDEVT = &p_sbdevt;

	/* initialize device values */

		P_DEVT->num_dev = 0;
		for (i = 0; i < D_MAX; i++)
			{
			(P_DEVT->dev_code[i]).opt_code = 0;
			(P_DEVT->dev_code[i]).rq_size = 0;
			(P_DEVT->dev_code[i]).cq_size = 0;
			(P_DEVT->dev_code[i]).boot_dev = 0;
			(P_DEVT->dev_code[i]).word_size = 0;
			(P_DEVT->dev_code[i]).brd_size = 0;
			(P_DEVT->dev_code[i]).smrt_brd = 0;
			(P_DEVT->dev_code[i]).cons_cap = 0;
			(P_DEVT->dev_code[i]).cons_file = 0;

			strcpy((P_DEVT->dev_code[i]).dev_name,"");
			}

	i = 0;

	/* assign values for devices */


		/* sbd board */

		(P_DEVT->dev_code[i]).opt_code = 1;
		(P_DEVT->dev_code[i]).rq_size = 0;
		(P_DEVT->dev_code[i]).cq_size = 0;
		(P_DEVT->dev_code[i]).boot_dev = 1;
		(P_DEVT->dev_code[i]).word_size = 1;
		(P_DEVT->dev_code[i]).brd_size = 1;
		(P_DEVT->dev_code[i]).smrt_brd = 1;
		(P_DEVT->dev_code[i]).cons_cap = 1;
		(P_DEVT->dev_code[i]).cons_file = 0;
		strcpy((P_DEVT->dev_code[i++]).dev_name,"SBD");

		/* ports board */

		(P_DEVT->dev_code[i]).opt_code = 3;
		(P_DEVT->dev_code[i]).rq_size = 3;
		(P_DEVT->dev_code[i]).cq_size = 35;
		(P_DEVT->dev_code[i]).boot_dev = 0;
		(P_DEVT->dev_code[i]).word_size = 1;
		(P_DEVT->dev_code[i]).brd_size = 0;
		(P_DEVT->dev_code[i]).smrt_brd = 1;
		(P_DEVT->dev_code[i]).cons_cap = 1;
		(P_DEVT->dev_code[i]).cons_file = 1;
		strcpy((P_DEVT->dev_code[i++]).dev_name,"PORTS");

		/* device entry count */

		P_DEVT->num_dev = i;
}


gensbdev()
{
	int i;
	DEV_TAB *P_DEVT;
	SBDEV_TAB *P_SBDEVT;

	P_DEVT = &p_devt;
	P_SBDEVT = &p_sbdevt;

	/*initialize subdevice entries */

		P_SBDEVT->num_sbdev=0;
		for (i = 0; i < SD_MAX; i++)
			{
			P_SBDEVT->sbdev_code[i].opt_code = 0;
			strcpy((P_SBDEVT->sbdev_code[i]).name,"");
			}

		i = 0;

		/* Null entry */

		P_SBDEVT->sbdev_code[i].opt_code = 0;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"NULL");

		/* 5 1/4" floppy disk */

		P_SBDEVT->sbdev_code[i].opt_code = 1;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"FD5");

		/* 10Mb winchester */

		P_SBDEVT->sbdev_code[i].opt_code = 2;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"HD10");

		/* 30Mb winchester */

		P_SBDEVT->sbdev_code[i].opt_code = 3;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"HD30");

		/* 72Mb CDC winchester */

		P_SBDEVT->sbdev_code[i].opt_code = 5;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"HD72");

		/* 72Mb Micropolis winchester */

		P_SBDEVT->sbdev_code[i].opt_code = 6;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"HD72A");

		/* 72Mb Priam winchester */

		P_SBDEVT->sbdev_code[i].opt_code = 7;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"HD72B");

		/* 72Mb Fujitsu winchester */

		P_SBDEVT->sbdev_code[i].opt_code = 8;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"HD72C");

		/* 43Mb Hitachi winchester */

		P_SBDEVT->sbdev_code[i].opt_code = 9;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"HD43");

		/* 72Mb Hitachi winchester */

		P_SBDEVT->sbdev_code[i].opt_code = 0x0a;
		strcpy((P_SBDEVT->sbdev_code[i++]).name,"HD72D");

		/* subdevice entry count */

		P_SBDEVT->num_sbdev = i;
}



dv_in()
{
int s;

if (terse != ON)
	printf("\nEnter device ID code: 0x");
if ((s = scanf ("%x",&opt_code)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("Enter device name: ");
if ((s = scanf ("%s",dev_name)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("Enter request queue size: 0x");
if ((s = scanf ("%x",&rq_size)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("Enter completion queue size: 0x");
if ((s = scanf ("%x",&cq_size)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("Boot device? (1 - yes / 0 - no): ");
if ((s = scanf ("%x",&boot_dev)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("16 bit I/O bus? (1 - yes / 0 - no): ");
if ((s = scanf ("%x",&word_size)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("Double width board? (1 - yes / 0 - no): ");
if ((s = scanf ("%x",&brd_size)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("Intelligent board? (1 - yes / 0 - no): ");
if ((s = scanf ("%x",&smrt_brd)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("Console capability? (1 - yes / 0 - no): ");
if ((s = scanf ("%x",&cons_cap)) == EOF || s == 0)
	return(s);
if (terse != ON)
	printf("Console pump file? (1 - yes / 0 - no): ");
s = scanf ("%x",&cons_file);

return(s);
}



sdv_in()
{
int s;

if (terse != ON)
	printf("\nEnter subdevice ID code: 0x");
if ((s = scanf ("%x",&opt_code)) == EOF || s == 0)
	return (s);
if (terse != ON)
	printf("Enter subdevice name: ");
if ((s = scanf ("%s",sdv_name)) == EOF || s == 0)
	return (s);
if (terse != ON)
	printf("Enter device name: ");
s = scanf ("%s",dev_name);

return(s);
}

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libxedt:libxedt.c	1.1"
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/edt.h>
#include <sys/extbus.h>
#include <sys/libxedt.h>
#include <sys/sys3b.h>
#include <sys/vtoc.h>


        /**************************************************************/
        /*********   It reads the system Equipped Device    ***********/
        /*********   Table (EDT) and the extended EDT's     ***********/
        /*********   and returns a pointer to a combined    ***********/
        /*********   partial EDT.  			    ***********/
        /**************************************************************/
     

/* Internal Functions */
struct libxedt *readedt();
int edtsize();
struct mainedt *getedt();
int getxedt();
	
struct mainedt *mainptr = NULL;
struct libxedt *useredt = NULL;

struct libxedt *
readedt ()
{
	struct mainedt *edtptr;
	int edtentries, part_size;
	int i, j, x;

	/* Get a pointer to the main EDT information */
	if((edtptr = getedt()) == NULL)
		return(useredt);

	/* Need to know how many entries there will be with both
	   the main edt and the xedts and malloc space for the new 
	   partial edt  */
	edtentries = 0;
	for(i=0; i < edtptr->edtsze.esize; ++i)  {
		edtentries++;
		if(edtptr->mainedt[i].indir_dev) 
			edtentries += edtptr->mainedt[i].n_subdev;
	}
				
	/* Malloc the space for the partial edt */
	part_size = (sizeof(struct partedt) * edtentries) + sizeof(int);
     	if((useredt = (struct libxedt *)malloc(part_size)) == NULL)
		return(useredt);


	/* Go thru each entry in the EDT and extended EDT's and
	   copy the name, major number, and board code to the 
	   partial EDT */
	for(i=0, x=0; i < edtptr->edtsze.esize; ++x, ++i)  {
		for(j=0; j<10; ++j)  {
			useredt->edt[x].dev_name[j] = edtptr->mainedt[i].dev_name[j];
		}
		useredt->edt[x].major = (unsigned long)edtptr->mainedt[i].opt_slot;
		useredt->edt[x].code = (unsigned long)edtptr->mainedt[i].opt_code;
		useredt->edt[x].parentmaj = 0;
		if(edtptr->mainedt[i].indir_dev)	{
			  x = getxedt(&edtptr->mainedt[i],x);
		}
	}
	useredt->edtentries = x;

	return(useredt);

}

int
edtsize()
{
	struct edtsze edt_size;
	int size;

     	if(sys3b(S3BEDT,&edt_size,sizeof(int)) != 0)
		return(0);
    
     	size = edt_size.esize * sizeof(struct edt) + edt_size.ssize 
		* sizeof(struct subdevice) + sizeof(int);

	return (size);
}

struct mainedt *
getedt()
{
	int edt_size;

	/* Find out the size of main EDT in order to malloc space
	   to read the main EDT into */
	if( (edt_size = edtsize()) == 0 )	{
		mainptr = NULL;
		return(mainptr);
	}

     	if((mainptr = (struct mainedt *)malloc(edt_size)) == NULL)
		return(mainptr);

     	if(sys3b(S3BEDT,mainptr,edt_size) != 0)
		mainptr = NULL;


	return(mainptr);

}

int 
getxedt(mainedt,x)
struct edt *mainedt;
int x;
{
	B_EDT *xedt = NULL;
	struct bedt_szinfo size_xedt;
	struct io_arg args;
	int xedtsize;
	char name[E_NAMLEN];
	char device[30];
	int lower;
	int fd, i, j, y;

	for (y = 0; y < E_NAMLEN && mainedt->dev_name[y] != '\0'; y++) {
			lower = tolower(mainedt->dev_name[y]);
			name[y] = lower;
	}
	name[y] = '\0';
	
	sprintf(device,"/dev/%s/c%d",name, mainedt->opt_slot);

	if ((fd = open( device, O_RDONLY)) < 0)
		return(x);

	/* Must read the first four chars of the xedt to get the
	   size fields  to determine how big the xedt is */
	args.memaddr = (unsigned long)&size_xedt;
	args.datasz = sizeof(struct bedt_szinfo);

	if (ioctl(fd, B_REDT, &args) < 0)
		return(x);

	/* malloc space to read the device's edt into */
	xedtsize = sizeof(B_EDT) + (size_xedt.max_tc * size_xedt.tc_size);
     	if((xedt = (B_EDT *)malloc(xedtsize)) == NULL)
		return(x);

	args.memaddr = (unsigned long)xedt;
	args.datasz = xedtsize;

	if (ioctl(fd, B_REDT, &args) < 0)	{
		return(x);
	}
	/* Create entry in the partial edt for each device in the
	   device's extended edt */
	for(i = 0; i < xedt->max_tc; ++i)	{
		if(xedt->TC_EQ(i)) {
			++x;
			useredt->edt[x].major = xedt->tc[i].maj;
			useredt->edt[x].parentmaj = mainedt->opt_slot;
			useredt->edt[x].code = 0;
			for(j=0; j<10; ++j)  {
				useredt->edt[x].dev_name[j] = xedt->tc[i].name[j];
			}
		}
	}
	return(x);

}

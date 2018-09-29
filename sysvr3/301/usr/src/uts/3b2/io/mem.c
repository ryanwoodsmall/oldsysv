/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/mem.c	10.6"
#include "sys/types.h"
#include "sys/sbd.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/firmware.h"
#include "sys/iobd.h"


/******************** THESE DEFINES TO REMOVED IN OI6 ********************/
#define VSIZEOFMEM (*(((struct vectors *)(BASE+0x20000))->p_memsize))

mmread(dev)
{
	register unsigned int n, po;

	dev = minor(dev);
	while (u.u_error == 0 && u.u_count != 0)  {
		n = min(u.u_count, ctob(1));

		/*
		 * read /dev/mem
		 */
		if (dev == 0) {

			/*	Don't try to read from mainstore
			**	we don't have.
			*/

			if (u.u_offset >= (VSIZEOFMEM) || u.u_offset < 0)
				u.u_error = ENXIO;
			else {
				po = u.u_offset % ctob(1);
				n = min(n,ctob(1) - po);
				if (copyout(u.u_offset + MAINSTORE,u.u_base,n))
					u.u_error = ENXIO;
			}

		/*
		 * read file /dev/kmem
		 */
		} else if (dev == 1) {

			/*	Don't try to read from I/O ports.
			**	This gives a system bus timeout
			**	if there is no board in the port.
			*/

			if (u.u_offset >= VKIO  &&
			   u.u_offset < VKIO + 12 * ctob(VKIOSZ))
				u.u_error = ENXIO;
			else {
				po = u.u_offset % ctob(1);
				n = min(n,ctob(1) - po);
				if (copyout(u.u_offset,u.u_base,n))
					u.u_error = ENXIO;
			}
		}
		else
			return;

		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}
}

mmwrite(dev)
{

	register unsigned int n, po;

	dev = minor(dev);
	while (u.u_error == 0 && u.u_count != 0)  {
		n = min(u.u_count, ctob(1));
		/*
		 * write /dev/mem
		 */
		if (dev == 0) {

			/*
			 * offset is beyond end of mainstore
			 */
			if (u.u_offset >= (VSIZEOFMEM) || u.u_offset < 0)
				u.u_error = ENXIO;

			else {
				po = u.u_offset % ctob(1);
				n = min(n, ctob(1) - po);
				if (copyin(u.u_base,u.u_offset + MAINSTORE,n))
					u.u_error = ENXIO;
			}

		/*
		 * write file /dev/kmem
		 */
		} else if (dev == 1) {

			/*	Don't try to write to I/O ports.
			**	This gives a system bus timeout
			**	if there is no board in the port.
			*/

			if (u.u_offset >= VKIO  &&
			   u.u_offset < VKIO + 12 * ctob(VKIOSZ))
				u.u_error = ENXIO;
			else {
				po = u.u_offset % ctob(1);
				n = min(n,ctob(1) - po);
				if (copyin(u.u_base,u.u_offset,n))
					u.u_error = ENXIO;
			}
		}
		
		u.u_base += n;
		u.u_count -= n;
		u.u_offset += n;
	}
}

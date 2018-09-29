/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)listen:listen.h	1.2"

/*
 * listen.h:	Include file for network listener related user programs
 *
 */

/*
 * The NLPS (Network Listener Process Service) 
 * protocol message sent by client machines to
 * a listener process to request a service on the listener's
 * machine. The message is sent to "netnodename(r_nodename)"
 * where r_nodename is the nodename (see uname(2)) of the
 * remote host. Note that client's need not know (or care)
 * about the details of this message.  They use the "nls_connect(3)"
 * library routine which uses this message.
 *
 * msg format:
 *
 * 		"id:low:high:service_code"
 *
 *		id = "NLPS"
 *		low:high = version number of listener (see prot msg)
 *		service_code is ASCII/decimal
 *
 * the following prot string can be run through sprintf with a service code
 * to generate the message:
 *
 *	len = sprintf(buf,nls_prot_msg,svc_code);
 *	t_snd(fd, buf, len, ...);
 *
 * See also:  listen(1), nlsrequest(3)
 *
 * and on the UNIX PC STARLAN NETWORK:
 * See also:  nlsname(3), nlsconnect(3), nlsestablish(3)
 */

static char *nls_prot_msg = "NLPS:000:001:%d";
static char *nls_v2_msg =   "NLPS:002:002:%s";

#define	NLSSTART	0
#define	NLSFORMAT	2
#define NLSUNKNOWN	3
#define NLSDISABLED	4



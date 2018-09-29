/* @(#)net.h	1.1 */
/*
**	Header for PCL-11B "net" command.
**
**	(interfaces "net" with "pcldaemon")
*/

char	buf[PCLBSZ+1];		/* communication buffer size */

/*
**	Format of CTRL message:
**
**	<user_name>' '<machine_id>' '<channel>' '<command...>
**
**	user_name:	login name from /etc/passwd
**	machine_id:	unique identifier extracted from end of "nodename"
**	channel:	numeral in range '0' to '7'
**	command:	passed to user login shell
*/

/*
**	Error messages from pcldaemon
*/

char	ack[]		= "acknowledgement";
#define	REPLYSIZE	((sizeof ack)&~1)
char	unk[REPLYSIZE]	= "user unknown";
char	ill[REPLYSIZE]	= "illegal access";
char	dir[REPLYSIZE]	= "bad directory";

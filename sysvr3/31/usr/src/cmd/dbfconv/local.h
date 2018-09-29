/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dbfconv:local.h	1.1"

/*
 * Current database version, MUST match nlsadmin and listen
 */

#define VERSION	2

/*
 * general defines
 */

#define VERSIONSTR	"# VERSION="
#define DBFLINESZ	BSIZE
#define DBFCOMMENT	'#'
#define DBFTOKENS	":#\n"
#define DEFAULTID	"listen"
#define RESERVED	"reserved"

/*
 * database structure, version 1
 */

struct	dbf_v1 {
	char	*dbf_svccode;
	char	*dbf_flags;
	char	*dbf_modules;
	char	*dbf_command;
	char	*dbf_comment;
};

/*
 * database structure, version 2
 */

struct	dbf_v2 {
	char	*dbf_svccode;
	char	*dbf_flags;
	char	*dbf_id;
	char	*dbf_reserved;
	char	*dbf_modules;
	char	*dbf_command;
	char	*dbf_comment;
};

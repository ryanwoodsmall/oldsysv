/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/msgfmts.c	1.6"
/*
**  LINTLIBRARY
*/

char	*_lp_msg_fmts[] =
{
    "",           	/* 0 - R_BAD_MESSAGE */
#if	defined(USE_FIFOS)
    "HSS",		/* 1 - S_NEW_QUEUE */
#else
    "LLS",        	/* 1 - S_NEW_QUEUE */
#endif
    "H",          	/* 2 - R_NEW_QUEUE */
    "H",          	/* 3 - S_ALLOC_FILES */
    "HS",         	/* 4 - R_ALLOC_FILES */
    "S",          	/* 5 - S_PRINT_REQUEST */
    "HSL",         	/* 6 - R_PRINT_REQUEST */
    "S",          	/* 7 - S_START_CHANGE_REQUEST */
    "HS",         	/* 8 - R_START_CHANGE_REQUEST */
    "S",          	/* 9 - S_END_CHANGE_REQUEST */
    "HL",         	/* 10 - R_END_CHANGE_REQUEST */
    "S",          	/* 11 - S_CANCEL_REQUEST */
    "H",          	/* 12 - R_CANCEL_REQUEST */
    "SSSSS",       	/* 13 - S_INQUIRE_REQUEST */
    "HSSLLHSSS",      	/* 14 - R_INQUIRE_REQUEST */
    "S",          	/* 15 - S_LOAD_PRINTER */
    "H",          	/* 16 - R_LOAD_PRINTER */
    "S",          	/* 17 - S_UNLOAD_PRINTER */
    "H",          	/* 18 - R_UNLOAD_PRINTER */
    "S",          	/* 19 - S_INQUIRE_PRINTER_STATUS */
    "HSSSSSHSLL", 	/* 20 - R_INQUIRE_PRINTER_STATUS */
    "S",          	/* 21 - S_LOAD_CLASS */
    "H",          	/* 22 - R_LOAD_CLASS */
    "S",          	/* 23 - S_UNLOAD_CLASS */
    "H",          	/* 24 - R_UNLOAD_CLASS */
    "S",          	/* 25 - S_INQUIRE_CLASS */
    "HSHSL",      	/* 26 - R_INQUIRE_CLASS */
    "SSS",        	/* 27 - S_MOUNT */
    "H",          	/* 28 - R_MOUNT */
    "SSS",        	/* 29 - S_UNMOUNT */
    "H",          	/* 30 - R_UNMOUNT */
    "SS",         	/* 31 - S_MOVE_REQUEST */
    "HL",          	/* 32 - R_MOVE_REQUEST */
    "SS",         	/* 33 - S_MOVE_DEST */
    "HSH",          	/* 34 - R_MOVE_DEST */
    "S",          	/* 35 - S_ACCEPT_DEST */
    "H",          	/* 36 - R_ACCEPT_DEST */
    "SS",         	/* 37 - S_REJECT_DEST */
    "H",          	/* 38 - R_REJECT_DEST */
    "S",          	/* 39 - S_ENABLE_DEST */
    "H",          	/* 40 - R_ENABLE_DEST */
    "SSH",         	/* 41 - S_DISABLE_DEST */
    "HS",          	/* 42 - R_DISABLE_DEST */
    "",          	/* 43 - S_LOAD_FILTER_TABLE */
    "H",          	/* 44 - R_LOAD_FILTER_TABLE */
    "",          	/* 45 - S_UNLOAD_FILTER_TABLE */
    "H",          	/* 46 - R_UNLOAD_FILTER_TABLE */
    "S",           	/* 47 - S_LOAD_PRINTWHEEL */
    "H",          	/* 48 - R_LOAD_PRINTWHEEL */
    "S",           	/* 49 - S_UNLOAD_PRINTWHEEL */
    "H",          	/* 50 - R_UNLOAD_PRINTWHEEL */
    "",           	/* 51 - S_LOAD_USER_FILE */
    "H",          	/* 52 - R_LOAD_USER_FILE */
    "",           	/* 53 - S_UNLOAD_USER_FILE */
    "H",          	/* 54 - R_UNLOAD_USER_FILE */
    "S",		/* 55 - S_LOAD_FORM */
    "H",		/* 56 - R_LOAD_FORM */
    "S",		/* 57 - S_UNLOAD_FORM */
    "H",		/* 58 - R_UNLOAD_FORM */
    "",			/* 59 - S_RFS_MOUNT */
    "H",		/* 60 - R_RFS_MOUNT */
    "SH",		/* 61 - S_QUIET_ALERT */
    "H",		/* 62 - R_QUIET_ALERT */
    "SLS",		/* 63 - S_SEND_FAULT */
    "H",		/* 64 - R_SEND_FAULT */
    "H",          	/* 65 - S_SHUTDOWN */
    "H",          	/* 66 - R_SHUTDOWN */
    "",			/* 67 - S_GOODBYE */
    "LHHH",		/* 68 - S_CHILD_DONE && LAST_UMESSAGE */
    "",           	/* 69 - I_GET_TYPE */
    "",			/* 70 - I_QUEUE_CHK && LAST_IMESSAGE */
    0,
};

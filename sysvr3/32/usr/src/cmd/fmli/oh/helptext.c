/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:oh/helptext.c	1.1"

char *Help_text[] = {

/***************************************************
 * CANCEL
 ***************************************************/
"\
\n\
\n\
   The CANCEL command allows you to cancel a current command or\n\
   activity.\n\
\n\
   To use CANCEL choose one of the following options:\n\
\n\
 	 - Screen Labeled Key - Press CANCEL\n\
\n\
 	 - Typed Command - Type cancel + RETURN\n\
",

/***************************************************
 * CLEANUP
 ***************************************************/
"\
\n\
   The CLEANUP command cancels all frames on the screen,\n\
   excluding those frames that can only be canceled upon EXIT. \n\
\n\
   Use one of the following:\n\
\n\
 	 - Command Menu - Select CLEANUP + RETURN\n\
\n\
 	 - Typed Command - Type cleanup + RETURN\n\
",

/***************************************************
 * CMD-MENU
 ***************************************************/
"\
The CMD-MENU is a multi-column menu of the commands available.\n\
It serves as a command reference list, a place from which you\n\
can issue a command, or a place to get HELP on a specific\n\
command.\n\
",
 
/***************************************************
 * EXIT
 ***************************************************/
"\
\n\
   Use the EXIT command to CANCEL all frames and exit the application.\n\
\n\
   To EXIT use one of the following options:\n\
\n\
 	 - Command Menu - Select EXIT + RETURN\n\
\n\
 	 - Typed Command - Type exit + RETURN\n\
",

/***************************************************
 * FRM-MGMT
 ***************************************************/
"\
\n\
   The FRM-MGMT command allows you to move and reshape frames.   It\n\
   also  supports  a  \\\"list\\\"  operation which lists for you all open\n\
   frames on the screen.  In the command syntax \\\"operation\\\"  refers\n\
   to \\\"MOVE\\\", \\\"RESHAPE\\\" or \\\"LIST\\\".\n\
\n\
     1.  To use the FRM-MGMT command, choose one of the following.\n\
	 If  a frame number or name is not specified, the currently\n\
	 active frame is assumed to be the frame you would like to\n\
	 operate  on.\n\
\n\
	    - Screen Labeled Key - Press FRM-MGMT\n\
\n\
	    - Command Menu - Select FRM-MGMT + RETURN\n\
\n\
	    - Typed Command - Type frm-mgmt [operation]\n\
			       [frame_#] + RETURN\n\
\n\
     2.  If an \\\"operation\\\" is not specified  a  pop-up  menu  frame\n\
	 will  appear  listing  the FRM-MGMT operations.  You select\n\
	 the frame operation you want  to  perform  by  moving  the\n\
	 cursor  to the item and pressing RETURN or the ENTER screen\n\
	 labeled  key.   Pressing  the  CANCEL  screen  labeled  key\n\
	 cancels the FRM-MGMT command.\n\
",

/***************************************************
 * GOTO
 ***************************************************/
"\
\n\
   The GOTO command allows you to move the cursor to another\n\
   open frame by specifying the frame number.\n\
\n\
     1.  To use the GOTO choose one of the following:\n\
\n\
	    - Command Menu - Select GOTO + RETURN\n\
\n\
	    - Typed Command - Type goto [frame_#] + RETURN\n\
\n\
     2.  If you use the GOTO command without specifying a frame\n\
	 number, you will be prompted for one.\n\
",

/***************************************************
 * HELP
 ***************************************************/
"\
\n\
   The HELP command creates an active frame containing a text message\n\
   which  describes either the action you are trying to perform or the\n\
   current object or form.  To exit the Help Facility press the CANCEL\n\
   screen labeled key.\n\
\n\
   To access HELP use one of the following options.  If no argument is\n\
   specified the system assumes you want help on the current frame or\n\
   on what you are currently doing.\n\
\n\
	 - Screen Labeled Key - Press HELP\n\
\n\
	 - Command Menu - Select HELP + RETURN\n\
\n\
	 - Typed command - Type help [command_name] + RETURN\n\
",

/***************************************************
 * NEXT-FRM
 ***************************************************/
"\
\n\
   The NEXT-FRM command allows you to move among frames according to\n\
   the order in which frames were opened.\n\
\n\
   To use NEXT-FRM choose one of the following options:\n\
\n\
 	 - Screen Labeled Key - Press NEXT-FRM\n\
\n\
 	 - Command Menu - Select NEXT-FRM + RETURN\n\
\n\
 	 - Typed Command - Type next-frm + RETURN\n\
",

/*************************************************** 
 * PREV-FRM
 ***************************************************/
"\
\n\
   The PREV-FRM command allows you to move to the previous frame\n\
   according to the reverse order in which the frames were opened.\n\
\n\
   To use PREV-FRM choose one of the following options:\n\
\n\
 	 - Screen Labeled Key - Press PREV-FRM\n\
\n\
 	 - Command Menu - Select PREV-FRM + RETURN\n\
\n\
 	 - Typed Command - Type prev-frm + RETURN\n\
",

/***************************************************
 * REFRESH
 ***************************************************/
"\
\n\
   The REFRESH command will simply redraw the screen.\n\
\n\
 	 - Command Menu - Select REFRESH + RETURN\n\
\n\
 	 - Typed Command - Type refresh + RETURN\n\
",

/***************************************************
 * UNIX
 ***************************************************/
"\
\n\
   The UNIX command invokes the UNIX shell.\n\
\n\
   To invoke UNIX use one of the following options:\n\
\n\
 	 - Command Menu - Select UNIX + RETURN\n\
\n\
 	 - Typed Command - Type unix + RETURN\n\
\n\
   Your screen will be cleared of frames and the \\\"UNIX:\\\" prompt  \n\
   will be displayed.\n\
\n\
   To return from the UNIX shell type \\\"exit\\\" at the \\\"UNIX:\\\" prompt.\n\
   You will then be prompted to press RETURN to REFRESH the screen.\n\
",


/***************************************************
 * UPDATE
 ***************************************************/
"\
\n\
   The UPDATE command is used to update the contents of a frame to\n\
   reflect changes made since the frame was  originally  opened.\n\
\n\
   To use the UPDATE command do one of the following options.  If a\n\
   frame  is  not  specified,  the current frame is assumed.  In the\n\
   \\\"typed command\\\" syntax the frame number is specified  as\n\
   it is listed in the label of the frame to be updated.  \n\
\n\
 	 - Command Menu - Select UPDATE + RETURN\n\
\n\
 	 - Typed Command - Type update [ frame_number] + RETURN\n\
",

};

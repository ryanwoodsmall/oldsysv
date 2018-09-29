/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)script:script.h	1.7"
#include <curses.h>
#include <signal.h>
#include <memory.h>
#include <fcntl.h>

#define MAXCHAR 128
#define MAXLINES 50
#define NCHAR 10

/******************************************************************
          Definitions of exit codes
******************************************************************/
#define BADCMD 3
#define PREVIOUS_MENU 20
#define TOP_MENU 21
#define EXIT_ASSIST 22
#define BREAK 23


/*****************************************************************************
    Definitions of names of script actions
******************************************************************************/
#define COMMENT 0
#define WAIT 1
#define MOVE 2
#define ADDSTR 3
#define ADDSTRCLR 4
#define INSTR 5
#define INSCH 6
#define ADDCH 7
#define MVADDSTR 8
#define MVADDSTRCLR 9
#define DELCH 10
#define CLRTOEOL 11
#define CLRTOBOT 12
#define REFRESH 15
#define REDRAW 16
#define MARK 18
#define MARKA 19
#define GETCH 20
#define GETSTR 21
#define FLUSHINP 25
#define EXITCH 26
#define DELETELN 27
#define INSERTLN 28
#define STANDOUT 30
#define STANDEND 31
#define BEEP 32
#define FLASH 33
#define ATTRON 34
#define ATTROFF 35
#define VERTLINE 50
#define BOX 58
#define FILLBOX 59
#define OPENFILE 100
#define MOREFILE 101
#define LOOPCH 200
#define LOOPSTR 210
#define LOOPSTRBYCH 220
#define NBAD 245
#define IFGOODORMAXBAD 246
#define IFMAXBADONLY 247
#define IFGOODONLY 248
#define IFNOTLASTCH 249
#define IFNOTLASTLOOP 251
#define IFNOTLASTBAD 252
#define DEFGOODSTR 253
#define DEFSAVESTR 254
#define ADDSAVESTR 255
#define DEFGOODCH 256
#define LOOPEND 250
#define CASE 271
#define DEFAULTCASE 272
#define CASEEND 280
#define SYSTEM 310
#define ADDUSRSTR 320
#define ADDHELPSTR 330
#define IFTERM 390
#define ELIFTERM 391
#define ELSETERM 392
#define TERMEND 393
#define SETSCRREG 400
#define SCROLLL 401
#define NSCROLL 402
#define IDLOK 410
#define SUBSTART 500
#define SUBEND 510
#define DOSUB 520
#define EXITFKEY 600
#define ADDCALLOUT 700
#define CLRCALLOUT 701
#define TEXT 705
#define TEXTEND 706
#define DONE 1000

/****************************************************************
    define keys
*****************************************************************/
#define CTRL_A 1
#define CTRL_D 4
#define CTRL_E 5
#define CTRL_L 12
#define CTRL_R 18
#define CTRL_T 20
#define CTRL_V 22
#define CTRL_Y 25
#define CTRL_Z 26


/******************************************************************************
    define minimum screen size 
*******************************************************************************/
#define NROWS 24
#define NCOLS 80



/******************************************************************************
    definitions for script subroutines
*******************************************************************************/
#define MAXSUBCHAR 14
#define MAXSUBS 50


/*******************************************************************
        Definitions for callouts
 ******************************************************************/
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define BEGIN 5
#define UPARROW '^'
#define DOWNARROW 'v'
#define RIGHTARROW '>'
#define LEFTARROW '<'

#define TOPLEFT 'l'
#define TOPRIGHT 'k'
#define BOTLEFT 'm'
#define BOTRIGHT 'j'
#define HOR 'q'
#define VERT 'x'



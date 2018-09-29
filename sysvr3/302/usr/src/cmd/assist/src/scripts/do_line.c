/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)script:do_line.c	1.10"
#include "script.h"
#include <ctype.h>

#define C_EQ_TESTC (usr_c==testc1)||(usr_c==testc2)||(usr_c==testc3)||(usr_c==testc4)||(usr_c==testc5)
#define N_TERM_ACTION action!=ELIFTERM&&action!=ELSETERM&&action!=TERMEND
#define NOT_CASE_ACTION action!=CASE&&action!=DEFAULTCASE&&action!=CASEEND&&action!=LOOPEND&&action!=SUBEND
#define NOT_ENDING_ACTION action!=CASEEND&&action!=LOOPEND&&action!=SUBEND

/*******************************************************************************
    definitions for parsing script lines containing strings
*******************************************************************************/
#define STRINGLINE  "%d %[]a-z A-Z0-9,<.>\/?;:\\\'\"\[\{\}\|\`~=+_)(*&^%$#@!-]"
#define STRINGLINE2  "%d %d %[]a-z A-Z0-9,<.>\/?;:\\\'\"\[\{\}\|\`~=+_)(*&^%$#@!-]"
#define STRINGLINE3  "%d %d %d %[]a-z A-Z0-9,<.>\/?;:\\\'\"\[\{\}\|\`~=+_)(*&^%$#@!-]"
#define FILELINE "%d %d %[]\[a-zA-Z0-9.*?$\/_-]"
#define CMP_THREE(A,B,C)  (tutstring[0]=='A'&&tutstring[1]=='B'&&tutstring[2]=='C')

#define BITS_PER_BYTE 8

extern int loopindex;
extern FILE  *fdtut;    /* fdtut points at script file */
extern char linebuf[MAXCHAR]; /* most recent script line */
extern int termalt, termso, termfkey;
extern struct termio termbuf;
extern char tempfile[];

int action = 0;            /* most recent script instruction; eg GETCH, REFRESH,) */
char svstring[MAXCHAR];
char usrstring[MAXCHAR];  /* most recent string entered by user to GETSTR */
int good_c1, good_c2, good_c3, good_c4, good_c5;
char good_str[MAXCHAR];
char loopbuf[MAXLINES][MAXCHAR];
int loopstate = FALSE;
char *stp = '\0' ;

int nsubs = 0;
int substate = FALSE;
long saveset;
struct seektbl {
    char subname[MAXSUBCHAR];
    long offset;    /* from beginning of scriptfile */
};
struct seektbl subtbl[MAXSUBS];

int usr_c = CTRL_A;     /* last character entered by user to GETCH instruction*/
int testc1, testc2, testc3, testc4, testc5;  /*used for case */
int exitch = CTRL_A;   /* calls walkthru commands menu */
int exitfkey = KEY_F(8);  /*calls walkthru commands menu */
int markrow, markcol;  /* row and col of last MARK instruction */
int markarow, markacol;  /* row and col of last MARKA instruction */
int alt_state = FALSE;
char *strcat();
do_line()
{
   char s1[MAXCHAR];
   char tempstr[BUFSIZ];
   char cmd1[MAXCHAR], cmd2[MAXCHAR];
   char err_line[MAXCHAR];
   int tutrow, tutcol;
   char tutstring[MAXCHAR];
   int kount, sysret, k1, k2, khar;
   int nchar, nlines,  nscroll;
   int nsec;
   char *q;

   int toprow, botrow;  /* for setscroll region */

   int nrows, ncols;    /* for box instruction */
   int cortl_c, cortr_c, corbl_c, corbr_c, row_c, col_c;
   int row_tl, col_tl, row_br, col_br;

   int maxloops;  /* number of loops */

   int termtest;
   FILE *fd1;
   

   sscanf(linebuf, "%d", &action);
          
   switch(action)   {

      case COMMENT:     /* a script comment */
	   break;

      case MOVE:  /* move cursor to row and col, which start counting at zero */
           sscanf(linebuf,"%d %d %d", &action, &tutrow, &tutcol);
           move(getrow(tutrow), getcol(tutcol));
           break;

      case REFRESH:
           refresh();
           ioctl(1,TCSBRK,1);    /* makes flushinp work better*/
           break;         

      case MARK:  /* marks (saves) current cursor position for later use */
           getyx(stdscr, markrow, markcol);
           break;

      case MARKA:  /* a second mark -  needed  to mark answers*/
           getyx(stdscr, markarow, markacol);
           break;

      case GETCH:  /* get a character from tty  */
           usr_c = getch();
           while (usr_c == exitch || usr_c == exitfkey) {
                helpmenu(usr_c);
                usr_c = getch();
           }
           if (usr_c == ERR) {
                done2();
                fprintf(stderr, "\nassist: mscript: input error ");
                fprintf(stderr, "at GETCH; program terminated.\n");
                exit(BADCMD);
           }
           break;

      case FLUSHINP:   /* flush input */
           flushinp();
           break;

      case MVADDSTR: /* move to position, then overwrite string */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf, STRINGLINE3, &action, &tutrow, &tutcol, tutstring);
           move(getrow(tutrow), getcol(tutcol));
           addstr(tutstring);
           break;

      case MVADDSTRCLR: /*move to position, the overwrite rest of line */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf,STRINGLINE3, &action, &tutrow, &tutcol, tutstring);
           mvaddstr(getrow(tutrow), getcol(tutcol), tutstring);
           clrtoeol();
           break;

      

      case DOSUB:  /* run subroutine */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf, " %d %s", &action, tutstring);
           saveset = ftell(fdtut);
           for(kount=0; (kount < nsubs) && 
                   (strcmp(tutstring,subtbl[kount].subname) != 0); kount++)  {
               ;
           }


           fseek(fdtut, subtbl[kount].offset, 0);
           break;

      case ADDSTR: /* string overwrite */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf,  STRINGLINE, &action, tutstring);
           addstr(tutstring);
           break;

      case ADDSTRCLR: /* string overwrite, deleting to end of line  */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf,  STRINGLINE, &action, tutstring);
           addstr(tutstring);
           clrtoeol();
           break;

      case ADDHELPSTR: /* add string giving ASSIST command keys for walkthrus*/
           addstr(unctrl(exitch));
           if(termfkey == TRUE) {
               addstr(" or f");
               addch(exitfkey-216);
           }
           break;

      case IFTERM: /* does actions up to ELIFTERM or ELSETERM or TERMEND
                      if terminal characteristics match specifications

                          A - if term has alternate char set
                          a - if term does not have alt char set
                          F - if term has function keys
                          f - if term does not have function keys
                          S - if term has standout mode
                          s - if term does not have standout mode

                     *****************************/


           memset(s1, '\0', MAXCHAR);
           sscanf(linebuf,"%d %s", &action, s1);
           termtest = getttest(s1);
           fgets(linebuf, MAXCHAR, fdtut);

           /* if the terminal has the capabilities specified  ***/
           if ( termtest == TRUE) {

               do_line();
               while(   ( N_TERM_ACTION)    &&
                         (fgets(linebuf, MAXCHAR, fdtut) != NULL) ) {
                    sscanf(linebuf,"%d", &action);
                    if (N_TERM_ACTION) {
                        do_line();
                    }
               }
               /* skip to end of terminal dependent lines */
               while ( ( action != TERMEND)  &&
                          (fgets(linebuf, MAXCHAR, fdtut) != NULL) ) {
                     sscanf(linebuf, "%d", &action);
               }
            }
           else {  /* non-match of terminal and characteristics  */
                sscanf(linebuf, "%d", &action);
                while (  (N_TERM_ACTION)  &&
                         (fgets(linebuf, MAXCHAR, fdtut) != NULL) )  {
                     sscanf(linebuf, "%d", &action);
                }
           }
           do_line();
           break;
          

           /** do this if IFTERM not matched  **/
      case ELIFTERM:
           sscanf(linebuf,"%d %s", &action, s1);
           termtest = getttest(s1);
           fgets(linebuf, MAXCHAR, fdtut);

           /* if the terminal has the capabilities specified  ***/
           if ( termtest == TRUE) {
               sscanf(linebuf, "%d", &action);
               do_line();
               while(   ( N_TERM_ACTION)    &&
                         (fgets(linebuf, MAXCHAR, fdtut) != NULL) ) {
                    sscanf(linebuf,"%d", &action);
                    if (N_TERM_ACTION) {
                        do_line();
                    }
               }
               /* skip to end of terminal dependent lines */
               while ( ( action != TERMEND)  &&
                          (fgets(linebuf, MAXCHAR, fdtut) != NULL) ) {
                     sscanf(linebuf, "%d", &action);
               }
            }
           else {  /* non-match of terminal and characteristics  */
                sscanf(linebuf, "%d", &action);
                while (  (N_TERM_ACTION)  &&
                         (fgets(linebuf, MAXCHAR, fdtut) != NULL) )  {
                     sscanf(linebuf, "%d", &action);
                }
           }
           do_line();
           break;
      


  
      case ELSETERM:
            if(fgets(linebuf, MAXCHAR, fdtut) != NULL) {
                 do_line();
            }
            while (  ( action != TERMEND )    &&
                      (fgets(linebuf, MAXCHAR, fdtut) != NULL) ) {
                 do_line();
            }
            break;

       case TERMEND:
            break;



      case INSTR:  /* insert string */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf,  STRINGLINE, &action, tutstring);
           getyx(stdscr, tutrow, tutcol);
           for(kount=0; tutstring[kount] != '\0';kount++) {
               insch(32);
           }
           move(tutrow, tutcol);
           addstr(tutstring);
           break;
      
      case INSCH:  /* insert char n times; character must be in digit form  */
           sscanf(linebuf,"%d %d %d", &action, &khar, &nchar);
           if (nchar < 1) {nchar = 1;}
           if(khar>127) khar = chgchar(khar,stp);
           if(khar == 532) addstr("SPACEBAR");
           else {
               while(nchar-- > 0) {
                   insch(32);
                   addch(khar);
               }
           }
           break;

      case ADDCH: /* overwrite n characters with khar; khar must be in digit form */
           sscanf(linebuf,"%d %d %d", &action, &khar, &nchar);
           if(nchar< 1) nchar = 1;
           if (khar > 127) khar = chgchar(khar,stp);
           if (khar == 532) addstr("SPACEBAR");
           else {
               while (nchar-- > 0) {
                   addch(khar);
               }
           }
           break;

      case DELCH:  /* delete n characters */
           sscanf(linebuf, "%d %d", &action, &nchar);
           while (nchar-- > 0) {
               delch();
           }
           break;

      case CLRTOEOL:  /* clear to end of line  */
           clrtoeol();
           break;

      case CLRTOBOT:   /* clear to bottom of screen  */
           clrtobot();
           break;

      case GETSTR: /* get a string from stdscr echoing string on screen */
           memset(usrstring, '\0', MAXCHAR);
           get_str(usrstring);
           break;
 
      case REDRAW:
           clearok(curscr);
           refresh();
           break;


      case DELETELN: /* delete line */
           deleteln();
           break;

      case INSERTLN:  /* insert line */
           insertln();
           break;

      case STANDOUT:  /* turn on standout - only works for curses add functions  */
           attrset(A_STANDOUT);
           break;
   
      case STANDEND:  /* turn off standout  */
           attrset(0);
           break;

      case BEEP:  /* sound beep */
           beep();
           break;

      case FLASH: /* flash screen or beep  */
           flash();
           break;

      case ATTRON:  /* turn on attribute - use at least first three lower-case
                   letters of attribute  */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf, "%d %s", &action, tutstring);
           if  CMP_THREE(a,l,t) {
               if(termalt==TRUE) {
                   alt_state = TRUE;
                   attron(A_ALTCHARSET);
               }
           }
           else if CMP_THREE(b,l,i)
                 attron(A_BLINK);
           else if CMP_THREE(b,o,l)
                 attron(A_BOLD);
           else if CMP_THREE(d,i,m)
                 attron(A_DIM);
           else if CMP_THREE(p,r,o)
                 attron(A_PROTECT);
           else if CMP_THREE(r,e,v)
                 attron(A_REVERSE);
           else if CMP_THREE(s,t,a)
                 attron(A_STANDOUT);
           else if CMP_THREE(u,n,d)
                 attron(A_UNDERLINE);
           else {
                 addstr("\nSCRIPT ERROR: Attribute ");
                 addstr(tutstring);
                 addstr(" incorrectly named\n");
                 refresh();
                 done2();
                 exit(BADCMD);
           }
           break;

       case ATTROFF: /* turn off attribute- same rules as case 34 */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf,"%d %s", &action, tutstring);
           if CMP_THREE(a,l,t) {
                  attroff(A_ALTCHARSET);
                  alt_state = FALSE;
           }
           else if CMP_THREE(b,l,i)
                 attroff(A_BLINK);
           else if CMP_THREE(b,o,l)
                 attroff(A_BOLD);
           else if CMP_THREE(d,i,m)
                 attroff(A_DIM);
           else if CMP_THREE(p,r,o)
                 attroff(A_PROTECT);
           else if CMP_THREE(r,e,v)
                  attroff(A_REVERSE);
           else if CMP_THREE(s,t,a)
                 attroff(A_STANDOUT);
           else if CMP_THREE(u,n,d)
                 attroff(A_UNDERLINE);
           else {
                 addstr("\nSCRIPT ERROR: Attribute ");
                 addstr(tutstring);
                 addstr(" incorrectly named\n");
                 refresh();
                 done2();
                 exit(BADCMD);
           }
           break;




      case OPENFILE:  /* open a file and read in first n lines  */
                      /* allows special file /tmp/twalk.$LOGNAME */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf, FILELINE, &action, &nlines, tutstring);
           if(strcmp(tutstring,"/tmp/twalk.$LOGNAME") == 0) {
               fd1 = fopen(tempfile, "r");
           }
           else fd1 = fopen(tutstring, "r");
           if(fd1 == NULL) {
               done2();
               sprintf(err_line,"assist: mscript:OPENFILE case: %s", tutstring);
               perror(err_line);
               exit(BADCMD);
           }
           getyx(stdscr, tutrow, tutcol);
           for(kount=0; kount < nlines  &&
                 (fgets(linebuf,MAXCHAR,fd1) != NULL); kount++)  {
               move(tutrow + kount, tutcol);
               addstr(linebuf);
           }
           break;

      case MOREFILE:  /* read in next m lines of opened file-only one file
                       can be opened at a time  */
           sscanf(linebuf,"%d %d", &action, &nlines);
           getyx(stdscr, tutrow, tutcol);
           for(kount=0;kount<nlines &&
                  (fgets(linebuf,MAXCHAR, fd1) != NULL); kount++)  {
               move(tutrow + kount, tutcol);
               addstr(linebuf);
           }
           break;
      /*
       *  while loops cannot be nested.  while loops          *
       *  cannot use subroutines.  subroutines can include    *
       *  non-nested while loops                              *
       */


      case DEFGOODCH: /* gets acceptable characters for loopch */
            good_c1 = good_c2 =good_c3 = good_c4 = good_c5 = 0;
            sscanf(linebuf, "%d %d %d %d %d %d", &action,
                    &good_c1, &good_c2, &good_c3, &good_c4, &good_c5);
            break;

      case LOOPCH:  /* loop - input character is matched against  */
                    /* good characters named in DEFGOODCH instruction */
            loopstate = TRUE;
            sscanf(linebuf, "%d %d", &action, &maxloops);
            saveloop();
            doloop(LOOPCH, maxloops);
            break;

      case DEFGOODSTR: /*gets string for loopstr and loopstrbych
                       put before loop start */
            memset(good_str, '\0', MAXCHAR);
            sscanf(linebuf, STRINGLINE, &action, good_str);
            break;
     
      case DEFSAVESTR:  /* gets a string that can be added later with ADDSAVESTR */
            memset(svstring, '\0', MAXCHAR);
            sscanf(linebuf, STRINGLINE, &action, svstring);
            break;

      case ADDSAVESTR: /* adds the string previously defined by DEFSAVESTR */
            addstr(svstring);
            break;

      case ADDUSRSTR: /* adds the string most previously entered by user */
            for(q=usrstring; isspace(*q) && *q!= '\0'; q++)  ;
            if(*q == '\0') sprintf(usrstring,"AN EMPTY LINE");
            getyx(stdscr, tutrow, tutcol);
            for(kount=0; usrstring[kount] != '\0'; kount++) ;
            if(tutcol + kount > NCOLS -1) {
                kount = NCOLS - 1 -tutcol -6 ;
                usrstring[kount++] = '<';
                usrstring[kount++] = 'M';
                usrstring[kount++] = 'O';
                usrstring[kount++] = 'R';
                usrstring[kount++] = 'E';
                usrstring[kount++] = '>';
                usrstring[kount++] = '\0';
            }
            addstr(usrstring);
            break;

      case LOOPSTR:  /* loop - input string is matched against*/
                     /* good string defined in DEFGOODSTR */
            loopstate = TRUE;
            sscanf(linebuf, "%d %d", &action, &maxloops);
            saveloop();
            doloop(LOOPSTR, maxloops);
            break;
           
      case LOOPSTRBYCH: /* loop -checks each character of string one at a */
                        /* time; correct string defined in DEFGOODSTR */
            loopstate = TRUE;
            sscanf(linebuf, STRINGLINE2, &action, &maxloops);
            saveloop();
            stp = good_str;
            while (*stp != '\0') {
                 doloop(LOOPSTRBYCH, maxloops);
                 stp++;
            }
            break;
    
      case NBAD: /* start of instructions for bad input */
            break;
      
      case IFGOODORMAXBAD:  /* start of instructions for good or maxbad input  */
            break;

      case IFMAXBADONLY: /* start of instructions for max bad only */
            break;

      case IFGOODONLY: /* start of instructions for good only input */
            break;

      case IFNOTLASTCH:  /* start of instructions only if not last char
                         *in string specified in case LOOPSTRBYCH
                         */
            break;

      case IFNOTLASTLOOP: /* use this to do a step for good
                           * or maxbad answer that you don't want to
                           * do the last loop - usually is a getch
                           */
            break;
      
      case IFNOTLASTBAD: /* use this for the steps that you want to do when
                          * answer is wrong and are going to let person try
                          * again, rather than script doing it for him 
                          * usually is getch or getstr
                          */
            ifnotlastbad();
            break;

      case LOOPEND:  /* end of while */
            loopstate = FALSE;
            break;
     
      case CASE: /* case match of character */
            testc1 = testc2 = testc3 = testc4 =testc5 = 0;
            sscanf(linebuf, "%d %d %d %d %d %d", &action, &testc1, &testc2,
                        &testc3, &testc4, &testc5);
            if(loopstate == FALSE) {
                fgets(linebuf, MAXCHAR, fdtut);
                if ( C_EQ_TESTC ) {

                     do_line();
                     while ( (NOT_CASE_ACTION) && 
                            (fgets(linebuf, MAXCHAR,fdtut) != NULL) ) {
                        sscanf(linebuf,"%d", &action);

                        if (NOT_CASE_ACTION) {
                            do_line();
                        }
                     }
                     while ( (NOT_ENDING_ACTION)  &&
                             (fgets(linebuf,MAXCHAR,fdtut) != NULL) ) {
                        sscanf(linebuf,"%d", &action);

                     }
                 }
                 else   {   /* non match of input char and test char  */
                      sscanf(linebuf, "%d", &action);

                      while( (NOT_CASE_ACTION) &&
                              (fgets(linebuf,MAXCHAR,fdtut)!=NULL) ) {
                         sscanf(linebuf,"%d",&action);

                      }
                 }
                 do_line();
             }
             else {    /*while state is true */
                 loopindex++;
                 strcpy(linebuf, loopbuf[loopindex]);



                 if(C_EQ_TESTC) {
                      do_line();
                      while( NOT_CASE_ACTION) {
                            loopindex++;
                            strcpy(linebuf, loopbuf[loopindex]);
                            sscanf(linebuf, "%d", &action);


                            if (NOT_CASE_ACTION) {
                                 do_line();
                            }
                       }
                       while (NOT_ENDING_ACTION) {
                             loopindex++;
                             sscanf(loopbuf[loopindex],"%d",&action);


                       }
                  }
                  else {
                       sscanf(linebuf,"%d", &action);


                       while (NOT_CASE_ACTION) {
                            loopindex++;
                            sscanf(loopbuf[loopindex],"%d", &action);


                       }
                  }
                  strcpy(linebuf,loopbuf[loopindex]);
                  do_line();
              }
           break;

      case DEFAULTCASE:  /*default of char switch-must come after other cases*/
           if(loopstate==FALSE) {
                while( (NOT_ENDING_ACTION) &&
                           fgets(linebuf, MAXCHAR, fdtut) !=NULL ) {


                     do_line();
                }
           }
           else  {  /*while state is true  */
                while (NOT_ENDING_ACTION) {
                    loopindex++;
                    strcpy(linebuf, loopbuf[loopindex]);


                    do_line();
                }
           }
           break;

      case CASEEND:  /* end of character switch  */
           break;

          
      
      case SYSTEM:  /* run any UNIX system call */
           memset(tutstring, '\0', MAXCHAR);
           memset(cmd1, '\0', MAXCHAR);
           memset(cmd2, '\0', MAXCHAR);
           sscanf(linebuf, STRINGLINE, &action, tutstring);
           saveterm();
           resetterm();
           updatetty(&termbuf);
           sscanf(tutstring,
               "%s %[]a-z A-Z0-9,<.>\/?;:\\\'\"\[\{\}\|\`~=+_)(*&^%$#@!-]",
                cmd1,cmd2);
           if (strcmp (strcat(cmd1, " "), "TUT ") == 0) {
                sprintf(tempstr,"%s", cmd2);
                sysret = system(tempstr) >> BITS_PER_BYTE;
                if (sysret==PREVIOUS_MENU || sysret==TOP_MENU
                     || sysret == EXIT_ASSIST ) {
                    done2();
                    exit(sysret);
                }
           }
           else system(tutstring);
           catchtty(&termbuf);
           fixterm();
           break;

      case TEXT:  /* print the following lines as text */
           sscanf(linebuf,"%d %d %d %d", &action, &tutrow, &tutcol, &kount);
           while(fgets(linebuf,MAXCHAR,fdtut) != NULL) {
               sscanf(linebuf, "%d", &action);
               if(action==TEXTEND) {
                    break;
               }
               else  {
                    move(getrow(tutrow++), getcol(tutcol) );
                    for(k1=0; linebuf[k1]!=10 && k1 < MAXCHAR; k1++, kount--) {
                       addch(linebuf[k1]);
                    }
                    while(kount-- > 0) {
                        addch(' ');
                    }
               }
           }
           break;
           

      case VERTLINE: /* draw a vertical line of character khar; khar must be
                  in digit form; line is nchar long */
           sscanf(linebuf,"%d %d %d", &action, &khar, &nchar);
           if (nchar < 1) nchar = 1;
           if (khar > 127) khar = chgchar(khar,stp);
           getyx(stdscr,tutrow,tutcol);
           while(nchar-- > 0) {
                addch(khar);
                move(tutrow++,tutcol);
           }
           break;

      case ADDCALLOUT: /* draws ASSIST style callouts- example
			  is addcallout d4 r2 a1 which draws 
			  a line 4 chars down then 2 right
			  and the adds one arrow head -
			  if altcharset is on and terminal has
			  alt char set, then uses altchar set
			  for lines */
           addcoch(alt_state);
	   callout(linebuf);
	   break;

      case CLRCALLOUT:  /* puts blanks at all specified places
                           for example, clrcallout d4 r3 clears
                           callout example above */
           clrcoch();
           callout(linebuf);
           break;


    
      case BOX:  

           sscanf(linebuf, "%d %d %d %d %d %d %d %d %d", &action,
                     &nrows, &ncols, &row_c, &col_c, &cortl_c, &cortr_c,
                     &corbl_c, &corbr_c);
           if(cortl_c =='\0') { cortl_c = cortr_c =corbl_c = corbr_c = ' ';}
           if(row_c == '\0') {
                row_c = 45;
                col_c = 124;
           }
           if(row_c == 601)  {
                row_c = chgchar(row_c,stp);
                col_c = chgchar(602,stp);
                cortl_c = chgchar(603,stp);
                cortr_c = chgchar(604,stp);
                corbl_c = chgchar(605,stp);
                corbr_c = chgchar(606,stp);
           }
                
           getyx(stdscr, tutrow, tutcol);
           addch(cortl_c);
           for (k2=1; k2< ncols -1 ; k2++) {
                addch(row_c);
           }
           addch(cortr_c);
           for (k1 = 1; k1 < nrows-1 ; k1++) {
                move(tutrow+k1,tutcol);
                addch(col_c);
                for (k2=1; k2< ncols-1; k2++) {
                     addch(32);
                }
                addch(col_c);
           }
           move(tutrow+k1, tutcol);
           addch(corbl_c);  
           for(k1 =1; k1< ncols-1; k1++)  {
                addch(row_c);
           }
           addch(corbr_c);
           break; 

      case FILLBOX:  /* filled box - makes a box filled with the
                   specified character - khar nust be in digit form */
           sscanf(linebuf, "%d %d %d %d %d %d", &action,
                   &row_tl, &col_tl, &row_br, &col_br, &khar);
           move(row_tl,col_tl);
           for(; row_br-row_tl > 0; row_tl++) {
               for(kount=0;kount < (col_br-col_tl); kount++) {
                    addch(khar);
               }
               move(row_tl+1,col_tl);
           }
           break;
                        


      case SETSCRREG:   /* set scrolling region in stdscr  */
	   sscanf(linebuf,"%d %d %d", &action, &toprow, &botrow);
           toprow = getrow(toprow);
           botrow = getrow(botrow);


	   setscrreg(toprow,botrow);
	   break;

      case SCROLLL:  /* scroll one line  */
	   scroll(stdscr);
	   break;


      case NSCROLL:  /* scroll n times  */
	   sscanf(linebuf, "%d %d ",&action,  &nscroll);
	   for(kount=0;kount<nscroll; kount++) {
	       scroll(stdscr);
	   }
	   break;

      case IDLOK:  /* set idlok to TRUE or FALSE */
           memset(tutstring, '\0', MAXCHAR);
           sscanf(linebuf, "%d %s", &action, tutstring);
           if (tutstring[0] == 't' || tutstring[0] == 'T') {
              idlok(stdscr, TRUE);
           }
           else if (tutstring[0] == 'f' || tutstring[0] == 'F') {
              idlok(stdscr, FALSE);
           }
           else {
           addstr("Script errors, case 410: idlok : true or");
           addstr("false not specified correctly\n");
           done2();
           exit(BADCMD);
           }
           break;

      case SUBSTART:  /* start of subroutine  */
           /* subroutine can include unnested while - whiles
               cannot include subroutines */
           sscanf(linebuf, "%d %s", &action, subtbl[nsubs].subname);
           subtbl[nsubs].offset = ftell(fdtut);
           nsubs++;


           while ( (action != SUBEND)  &&
                   (fgets (linebuf, MAXCHAR, fdtut) != NULL)  )  {
              sscanf(linebuf, "%d", &action);
           }
           break;

      case SUBEND:
           fseek(fdtut, saveset, 0);
           break;
      case WAIT:  /* wait */
           sscanf(linebuf, "%d %d", &action, &nsec);
           sleep(nsec);
           break;

      case EXITCH:   /* define exit character in digit form  */
           sscanf(linebuf, "%d %d", &action, &exitch);
           break;

      case EXITFKEY:   /* define exit function key in digit form  */
           sscanf(linebuf, "%d %d", &action, &exitfkey);
           break;

      case DONE:
           sscanf(linebuf, "%d %d", &action, &k1);
           if(k1==NULL) k1 = 0;
	   done2();
           exit(k1);
	   break;

      default:
           addstr("\nUNDEFINED ACTION: ");
           addstr(linebuf);
           refresh();
           break;

   }
   return(0);
}



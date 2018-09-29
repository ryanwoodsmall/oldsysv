/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)script:utilities.c	1.10"
#include "script.h"

#define NO_CONDITION_ACTION  (action!=246&&action!=247&&action!=248&&action!=249&&action!=250&&action!=251)

/******************************************************************************
    define values for relative cursor moves
******************************************************************************/
#define REL_VAL 1000  /* current cursor position */
#define REL_ROW_MIN 976    /*allows neg. moves relative to current cursor posiiton */
#define REL_COL_MIN 920
#define MARK_VAL 2000   /* marked cursor position */
#define MARK_ROW_MIN 1976  /* allows for neg. moves relative to mark  */
#define MARK_COL_MIN 1920
#define MARKA_VAL 3000  /* a second marker  */
#define MARKA_ROW_MIN 2976
#define MARKA_COL_MIN 2920

extern int done1(), done2(), bdone1(), bdone2();
extern char *tmpscript;
extern int buildflag, menuflag;
extern FILE  *fdtut;    /* fdtut points at script file */
extern char linebuf[MAXCHAR]; /*most recent script line */
extern int action;
extern WINDOW *helpwin;  /* walkthru commands */
extern int usr_c;  /* last character entered by user to GETCH instruction */
extern char usrstring[MAXCHAR];  /* last string entered by user to GETSTR */
extern int good_c1, good_c2, good_c3, good_c4, good_c5;
extern char good_str[MAXCHAR];
extern char loopbuf[MAXLINES][MAXCHAR];
extern int loopstate;
extern char *stp;
extern int exitch, exitfkey;
extern int markrow, markcol, markarow, markacol;
extern int termalt, termso, termfkey;
extern int alt_state;
extern struct termio termbuf;
extern char tempfile[];

int loopindex;
int nmaxbad; 
int nbad;  /* number of times user is allowed to enter wrong answer*/
/*
 * Clean up  for ending
 */
done1()
{
    if (buildflag == TRUE) unlink(tmpscript);
}

done2()
{
    signal(SIGINT, SIG_IGN);
    if (buildflag == TRUE) unlink(tmpscript);
    fclose(fdtut);
    attrset(0);                     /* set attributes to off */
    move(LINES-1,0);		/* to lower left corner */
    clrtoeol();			/* clear bottom line */
    refresh();			/* flush out everything */
    endwin();			/* curses cleanup */
    updatetty(&termbuf);        /* put in most recent stty settings */
    unlink(tempfile);           /* Dispose of temporary file used in script */
}


/*
 * endings for break; 
 */

bdone1()
{
        int signo;
	for(signo=1; signo< 18; signo++) signal(signo, SIG_IGN);
        if(buildflag == TRUE && (access(tmpscript,00) == 0) ) unlink(tmpscript); 
        exit(BREAK);
}


bdone2()
{
        int signo;
	for(signo=1; signo< 18; signo++) signal(signo, SIG_IGN);
        if(LINES > 1) {
            move(LINES-1,0);		/* to lower left corner */
	    clrtoeol();			/* clear bottom line */
            attrset(0);
	    refresh();		/* flush out everything */
        }
	endwin();			/* curses cleanup */
        if(buildflag == TRUE && (access(tmpscript,00) == 0) ) unlink(tmpscript); 
        fclose(fdtut);
        updatetty(&termbuf);
        unlink(tempfile);  
	exit(BREAK);
}

/*
 *   save the lines of the loop in loopbuf[][]
 */
saveloop()
{
     int kount;
     for(kount=0; (action != LOOPEND)  &&
                 (fgets(linebuf, MAXCHAR, fdtut) != NULL); kount++) {
        sscanf(linebuf, "%d", &action);


        strcpy(loopbuf[kount],linebuf);


     }



}

doloop(loopaction, maxlp)
int loopaction;
int maxlp;  /* number of loops */
{
     int nloops;
     for(nloops = 0; nloops < maxlp; nloops++)  {
        sscanf(loopbuf[0], "%d %d", &action, &nmaxbad);
        if( nonmatch(loopaction) ) {
           for(nbad=0; (nbad< nmaxbad) && nonmatch(loopaction) ; nbad++) {
               loopindex = 1;
               sscanf(loopbuf[loopindex],"%d", &action);
               while( (NO_CONDITION_ACTION) && (nonmatch(loopaction)) ) {
                  strcpy(linebuf,loopbuf[loopindex]);
                  do_line();
                  loopindex++;
               }
            }
         }
         /*
          * Now skip over the actions done for wrong entries and 
          * less than max bad 
          */
         for( loopindex=1; NO_CONDITION_ACTION; loopindex++)  {
            sscanf(loopbuf[loopindex], "%d", &action);
         }
         while (action != LOOPEND) {
            switch(action)  {

               case IFGOODORMAXBAD:
                   if( (!nonmatch(loopaction)) || (nbad == nmaxbad) ) {
                           wh_part2();
                   }
                   else {
                        sscanf(loopbuf[loopindex],"%d",&action);
                        while (NO_CONDITION_ACTION) {
                            sscanf(loopbuf[loopindex],"%d",&action);
                            loopindex++;
                        }
                   }
                   break;

               case IFNOTLASTLOOP:
                   if((!nonmatch(loopaction)||nbad==nmaxbad)&&(nloops<maxlp-1)){
                         wh_part2();
                   }
                   else {
                        sscanf(loopbuf[loopindex],"%d",&action);
                        while (NO_CONDITION_ACTION) {
                            sscanf(loopbuf[loopindex],"%d",&action);
                            loopindex++;
                        }
                   }
                   break;
               
               case IFMAXBADONLY:
                   if( (nbad == nmaxbad) && (nonmatch(loopaction)) ) {
                           wh_part2();
                   }
                   else {
                        sscanf(loopbuf[loopindex],"%d",&action);
                        while (NO_CONDITION_ACTION) {
                            sscanf(loopbuf[loopindex],"%d",&action);
                            loopindex++;
                        }
                   }
                   break;

               case IFGOODONLY:
                    if( (!nonmatch(loopaction)) && (nbad <= nmaxbad) ) {
                           wh_part2();
                    }
                   else {
                        sscanf(loopbuf[loopindex],"%d",&action);
                        while (NO_CONDITION_ACTION) {
                            sscanf(loopbuf[loopindex],"%d",&action);
                            loopindex++;
                        }
                   }
                    break;
               
                case IFNOTLASTCH: /* use with loopaction ==LOOPSTRBYCH only */
                   if( stp[1] != '\0') {
                              wh_part2();
                   }
           
                   else {
                        sscanf(loopbuf[loopindex],"%d",&action);
                        while (NO_CONDITION_ACTION) {
                            sscanf(loopbuf[loopindex],"%d",&action);
                            loopindex++;
                        }
                   }
                    break;
               
                            
               default:
                   done2();
                   fprintf(stderr, "\nmscript: Wrong action in doloop()\n");
                   exit(BADCMD);
            }
         }
      }
      loopstate = FALSE;
}
/*
 * see if users answer matched the answer specified in script
 */
nonmatch(loopact)
int loopact;
{
    switch(loopact) {
        case LOOPCH:
             if( usr_c != good_c1 && usr_c != good_c2 && usr_c != good_c3
                && usr_c != good_c4 && usr_c != good_c5) {
                 return(TRUE);
             }
             else return(FALSE);
             break;
        case LOOPSTR:
             if (str_cmp(good_str, usrstring) != 0)
                  return(TRUE);
             else return(FALSE);  /*usr input matched correct string*/
             break;
        case LOOPSTRBYCH:
             if(usr_c != *stp)
                  return(TRUE);
             else return(FALSE);
             break;
        default:
             done2();
             fprintf(stderr, "mscript:nonmatch() Incorrect loopact \n");
             exit(BADCMD);
     }
}


      
 /* use this for the steps that you want to do when
  * answer is wrong and are going to let person try
  * again, rather than script doing it for him 
  * usually is getch or getstr
  * kount is number of lines to skip on last bad loop
  */
ifnotlastbad()
{
    int kount;
    if(nbad==nmaxbad-1)  {
       sscanf(linebuf,"%d %d", &action, &kount);
       loopindex = loopindex + kount;
    }
}


/*
 * string compare routine ptr1 points at specified correct string 
 * and ptr2 at what the user entered
 */

str_cmp(ptr1,ptr2)
char *ptr1, *ptr2;
{
     while(*ptr1 != '\0') {
         while (*ptr1 == '%') {
              ptr1++;
              switch(*ptr1) {
                  case '0':  /* 0 or more white space */
                     while ( (*ptr2 == ' ') || (*ptr2 == '\t') ) {
                           ptr2++;
                     }
                     break;
       
                  case '1':  /* 1 or more white space  */
                      if( (*ptr2 != ' ') && (*ptr2 != '\t') ) {
                           return(1);
                      }
                      while ( (*ptr2 == ' ') || (*ptr2 == '\t') ) {
                           ptr2++;
                      }
                      break;
    
                  case '2':  /* accept 0 or 1 of the following character */
                      ptr1++;
                      if (*ptr2 == *ptr1) ptr2++;
                      break;
 
                  case '3':  /* accept 0 or more of the following character */
                      ptr1++;
                       while(*ptr2 == *ptr1) {
                             ptr2++;
                       }
                       break;

                  case '4':  /* accept 1 or more of the following character */
                       ptr1++;
                       if( *ptr1 != *ptr2) return(1);
                       while (*ptr2 == *ptr1) {
                              ptr2++;
                       }
                       break;
                   case '%':
                      if(*ptr2 == '%') {
                           ptr2++;
                      }
                      else return(1);
                      break;

                   default:
                      done2();
                      fprintf(stderr,"mscript: str_cmp()incorrect defgoodstr format.\n");
                      fprintf(stderr,"\nTranslated line of script is: %s ", linebuf);
                      exit(BADCMD);
                      break;
                }
                ptr1++;
           }
           if (*ptr1 != *ptr2) {
               return(1);
            }
           else {
                ptr1++;
                ptr2++;
           }
      }
      if (*ptr2 != '\0') {
         return(1);
      }
      else return(0);
}
            

wh_part2()
{
       sscanf(loopbuf[loopindex],"%d",&action);
       while(NO_CONDITION_ACTION) {
           strcpy(linebuf, loopbuf[loopindex]);
           sscanf(linebuf, "%d", &action);
           do_line();
           loopindex++;
       }
}



/*
 * walkthru commands
 * The commands are invoked with ^A and f8 unless the current script
 * changes the exitch or exitfkey
 */
void helpmenu(ex_ch)  
int ex_ch;
{
    int hc;
    char syscmd[BUFSIZ];
    WINDOW *syswin;
    flushinp();
    touchwin(helpwin);
    if (ex_ch == exitfkey ) fkeyhelp();
    else  ckeyhelp();
    wrefresh(helpwin);
    hc = wgetch(helpwin);
    while ( hc!=CTRL_D && hc!=CTRL_E &&hc!=CTRL_L  && hc!= CTRL_R
              && hc!= CTRL_T && hc!= CTRL_V
              && hc!=KEY_F(2) && hc!=KEY_F(3) && hc!=KEY_F(4)  ) {
       if ( hc == CTRL_A) {ckeyhelp(); wrefresh(helpwin);}
       else if (hc == KEY_F(8)) {fkeyhelp(); wrefresh(helpwin); }
       else beep();
       hc=wgetch(helpwin);
    }
    if( hc== CTRL_L || hc == KEY_F(3) || hc == CTRL_V || hc ==KEY_F(2) ) {  
        /* Redraw screen and/or clear help message  */
         clearok(curscr,TRUE);
         touchwin(stdscr);
         refresh();
    }
    else if ( hc == CTRL_E) {   /* shell escape */
         syswin = newwin(LINES,COLS,0,0);
         touchwin(syswin);
         mvwaddstr(syswin,0,0,"UNIX SYSTEM COMMAND: ");
         wrefresh(syswin);
         echo();
         nl();
         wgetstr(syswin,syscmd);
         touchwin(stdscr);
         saveterm();
         resetterm();
         updatetty(&termbuf);
         system(syscmd);
         catchtty(&termbuf);
         printf("\n--> Strike any character to return to the walkthru " );
         nonl();
         noecho();
         wgetch(syswin);
         fixterm();
         clearok(curscr,TRUE);
         touchwin(stdscr);
         refresh();
            
    }

    else {
        switch(hc) {
            case CTRL_R:   
            case KEY_F(4): 
                done2();
                exit(PREVIOUS_MENU);
                break;
            case CTRL_T: 
                done2();
                exit(TOP_MENU);
                break;
            case CTRL_D:
                done2();
                exit(EXIT_ASSIST);
                break;
            default:
                done2();
                fprintf(stderr, "assist: mscript:helpmenu():non-existent walkthru key\n");
                exit(BADCMD);
                break;
         }
     }
}

ckeyhelp()
{
            mvwprintw(helpwin, 1, 29, "WALKTHRU COMMANDS");
            mvwprintw(helpwin, 3, 2, " NAVIGATING, EXECUTING:      CURSOR POSITIONING:    MISCELLANEOUS:");
            mvwprintw(helpwin, 4, 2, " ^G ...                      <CR>,^N ...            ^L Redraw screen");
            mvwprintw(helpwin, 5, 2, " ^V Clear message                 ^P ...            ^Y ...");
            if (menuflag == TRUE) mvwprintw(helpwin, 6, 2, " ^R Previous menu                 ^W ...            ^O ...");
            else                  mvwprintw(helpwin, 6, 2, " ^R ...                           ^W ...            ^O ...");

            mvwprintw(helpwin, 7, 2, " ^F ...                           ^B ...            ^A Walkthru Commands");

            if (menuflag == TRUE) mvwprintw(helpwin, 8, 2, " ^T Top menu                      ^U ...            ^K ...     ");
            else                  mvwprintw(helpwin, 8, 2, " ^T ...                           ^U ...            ^K ...     ");

            if (menuflag == TRUE) mvwprintw(helpwin, 9, 2, " ^D Exit ASSIST                                     ^C ...  ");
            else                  mvwprintw(helpwin, 9, 2, " ^D Exit Walkthru                                   ^C ...  ");
            mvwprintw(helpwin, 10, 2, " ^E Shell escape ");
            wattron(helpwin,A_STANDOUT);
            mvwprintw(helpwin, 11, 14, " TYPE WALKTHRU COMMAND, OR TYPE ^V TO CLEAR THIS MESSAGE ");
            wattroff(helpwin, A_STANDOUT);
}

fkeyhelp()
{
            mvwprintw(helpwin, 1, 29, "WALKTHRU COMMANDS");
            mvwprintw(helpwin, 3, 2, " NAVIGATING, EXECUTING:    CURSOR POSITIONING:      MISCELLANEOUS:");
            mvwprintw(helpwin, 4, 2, " f1 ...                    <CR>,DOWN ...            f3 Redraw screen");
            mvwprintw(helpwin, 5, 2, " f2 Clear help                    UP ...            f6 ...     ");

            if (menuflag == TRUE) mvwprintw(helpwin, 6, 2, " f4 Previous menu              RIGHT ...            f7 ... ");
            else                  mvwprintw(helpwin, 6, 2, " f4 ...                        RIGHT ...            f7 ... ");

            mvwprintw(helpwin, 7, 2, " f5 ...                         LEFT ...            f8 Walkthru Commands");
            if (menuflag == TRUE) mvwprintw(helpwin, 8, 2, " ^T Top menu                      ^U ...            ^K ...     ");
            else                  mvwprintw(helpwin, 8, 2, " ^T ...                           ^U ...            ^K ...     ");

            if (menuflag == TRUE) mvwprintw(helpwin, 9, 2, " ^D Exit ASSIST                                     ^C ...  ");
            else                  mvwprintw(helpwin, 9, 2, " ^D Exit Walkthru                                   ^C ...  ");
            mvwprintw(helpwin, 10, 2, " ^E Shell escape ");
            wattron(helpwin,A_STANDOUT);
            mvwprintw(helpwin, 11, 14, " TYPE WALKTHRU COMMAND, OR TYPE f2 TO CLEAR THIS MESSAGE ");
            wattroff(helpwin, A_STANDOUT);
}

/*
 * routine to obtain absolute values of cursor row 
 * from current cursor position or to marked (saved)
 * cursor position
 */
getrow(anyrow)
int anyrow;
{
     int  temprow, tempcol;
     if(anyrow >= REL_ROW_MIN )  {
         getyx(stdscr, temprow, tempcol);
         if( anyrow < MARK_ROW_MIN ) {
              anyrow = anyrow - REL_VAL + temprow;
         }
         else  if ( anyrow < MARKA_ROW_MIN) {
              anyrow = anyrow - MARK_VAL + markrow;
         }
         else anyrow = anyrow - MARKA_VAL + markarow;
     }


     return(anyrow);
}
              
/*
 * routine to obtain absolute value of cursor col
 * from current cursor position or to marked (saved)
 * cursor position
 */
getcol(anycol)
int  anycol;
{
     int tempcol, temprow;
     if( anycol >= REL_COL_MIN) {
         getyx(stdscr, temprow, tempcol);
         if( anycol < MARK_COL_MIN ) {
              anycol = anycol - REL_VAL + tempcol;
         }
         else if (anycol < MARKA_COL_MIN)  {
              anycol = anycol - MARK_VAL + markcol;
         }
         else anycol = anycol - MARKA_VAL + markacol;
     
     }


     return(anycol);
}
              
get_str(anystr)
char *anystr;
{
     int myerase, mykill, kount, maxkount;
     char *cp;
     int colstart;
     int str_row, str_col;
     myerase = erasechar();
     mykill = killchar();
     kount = 0;
     memset(anystr, '\0', MAXCHAR);
     cp = anystr;

     if(sscanf(linebuf,"%d %d", &action, &maxkount) < 2) maxkount = 77;
     getyx(stdscr, str_row, str_col);
     colstart = str_col;
     usr_c = getch();
     for(;;) {
          if (usr_c == 13 || usr_c== 10) {
                *cp = '\0';
                break;
          }
          if (usr_c == myerase || usr_c == 8 ) {
               if (str_col > colstart) {
                    str_col--;
                    move(str_row, str_col);
                    addch(32); 
                    move(str_row, str_col);
                    refresh();
                    *cp = '\0';
                     cp--;
                    kount--;
               }
               else beep();
          }
          else if (usr_c == mykill) {
               memset(anystr, '\0', MAXCHAR);
               while(str_col > colstart) {
                    str_col--;
                    move(str_row, str_col);
                    addch(32);
                    move(str_row, str_col);
               }
               refresh();
               cp = anystr;
               kount = 0;
          }
          else if (usr_c==exitch || usr_c == exitfkey) {  /* ^A or f8 defaults */
               helpmenu(usr_c);
          }
          else if (kount >= maxkount) beep();
          else if (usr_c < 32 || usr_c> 126)  {  /* if usr_c is a special character */
               beep();
          }
          else if (usr_c == ERR) {
               done2();
               fprintf(stderr, "assist: mscript: Input error at get_str(); program terminated.\n");
               exit(BADCMD);
          }
          else {
               addch(usr_c);
               refresh();
               str_col++;
               *cp = usr_c;
               cp++;
               kount++;
          }
          usr_c = getch();
      }
     return(0);
}

/*
 * Routine to process the special character values ASSIST scripts
 * can use.  500 is the character pointed at in loopstrbych; 
 * 501 is last character entered by user to a getch instruction;
 * 510 is the same as 500 except that a space is printed out; this
 * is used to tell person to type a space
 * 601-606 are the alternate characters that draw nice horizontal
 * and vertical lines abd nice boxes, when the terminal has an
 * alternate graphical set (turned on with attron alt)
 */
chgchar(kh,p)
int kh;
char *p;
{
     if(kh==500||kh==510) {
         if(*p ==' ' && kh == 510) kh = 532;
         else kh = *p;
         usr_c = 0;  /* fixes a dumb bug - but I don't know why 
                  * bug is that if 500 is specified, kh is not
                  * shown on screen if it was last char from getch. 
                  */
     }
     else if(kh==501) { kh = usr_c; }
     else if (termalt==TRUE && alt_state==TRUE) {
         switch(kh) {
             case 601: 
                  kh = HOR;
                  break;
             case 602:
                  kh = VERT; 
                  break;
             case 603:
                  kh = TOPLEFT;  /* top-left cornercharacter */
                  break;
             case 604:
                  kh = TOPRIGHT;
                  break;
             case 605:
                  kh = BOTLEFT;
                  break;
             case 606:
                  kh = BOTRIGHT;
                  break;
         }
     }
     else {
         switch(kh) {
             case 601:
                  kh = '-';
                  break;
             case 602:
                  kh = '|';
                  break;
             default:
                  kh = ' ';
                  break;
         }
     }
     return(kh);
}
   
catchtty(tbuf)
struct termio *tbuf;
{
    ioctl(fileno(stdin), TCGETA, tbuf);
}

updatetty(tbuf)
struct termio *tbuf;
{
    ioctl(fileno(stdin), TCSETAW, tbuf);
}

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)script:mkscript.c	1.7"
#include "script.h"
extern char *tmpscript;
char linename[MAXCHAR];
char buildline[MAXCHAR];
char firstc;
mkscript(subj, subsubj, mpath)
char *subj, *subsubj, *mpath;
{
    char *tempnam(), *pfx1;
    int foundname = FALSE;
    char *tp;
    FILE *fp;
    char tmpstr[BUFSIZ];
    char tempdir[NCHAR];
    char *tmpdir;
    char prefix1[NCHAR];
    char err_line[MAXCHAR];


    sprintf(tmpstr,"%s/%s.build", mpath,subj);
    fp = fopen(tmpstr, "r");
    if(fp == NULL) {
        sprintf(err_line, "assist: mscript: mkscript: %s", tmpstr);
    	perror(err_line);
    	exit(BADCMD);
    }


    sprintf(tempdir,"/tmp");  /* put temporary copy of script in /tmp */
    sprintf(prefix1,"walk");

    while( foundname== FALSE &&(fgets(buildline, MAXCHAR, fp) != NULL) ) {
    	sscanf(buildline,"%c %s ", &firstc, linename);
    	if( (firstc == '') && (strcmp(subsubj,linename)==0) ) {
    		foundname = TRUE;
    	}
    }
    if(foundname == FALSE) {
    	fprintf(stderr,"assist: mscript: could not find walkthru ");
        fprintf(stderr, "sub-subject %s in %s.build file\n", subsubj, subj);
    	exit(BADCMD);
    }
       
    tmpdir = tempdir;
    pfx1 = prefix1;
    tmpscript = tempnam(tmpdir, pfx1);

    firstc = 0;

/*
 *  mscript uses the commands "cat" and "pcat" to build its scripts
 *  from the pieces of scripts.  mscript will exit here if either
 *  command is not in its expected location or if either is non-
 *  executable.
 */
    if (access("/bin/cat",01) != 0) {
        perror("assist: mscript: /bin/cat");
        done1();
        exit(BADCMD);
    }
    if(access("/usr/bin/pcat",01) != 0){
        perror("assist: mscript: /usr/bin/pcat");
        done1();
        exit(BADCMD);
    }

    while( ( firstc != CTRL_Z) && (fgets(buildline,MAXCHAR,fp) != NULL) ) {
       sscanf(buildline,"%c %s", &firstc, linename);
       if (firstc != CTRL_Z) {
           if (firstc == CTRL_Y) {
                getfname();
           }
           for(tp = linename; *tp!='\0'; tp++) {;}
           if( (*(tp-1) == 'z') && (*(tp-2) == '.')) { /*packed file*/
                sprintf(tmpstr, "/usr/bin/pcat %s/%s >> %s ", 
                          mpath,linename, tmpscript);
                if (system(tmpstr) != 0)  {
                    sprintf(err_line, "assist: mscript: system( %s )", tmpstr);
                    perror(err_line);
                    done1();
                    exit(BADCMD);
                }
            }
            else if (linename[0] != '\0') {     /* file is not packed */
                sprintf(tmpstr,"/bin/cat %s/%s >> %s", mpath,
                              linename, tmpscript);
                if (system(tmpstr) != 0) {
                    sprintf(err_line, "assist: mscript: system( %s )", tmpstr);
                    perror(err_line);
                    done1();
                    exit(BADCMD);
                }
            }
        }
    }
}

getfname()
{
   FILE *fpshell;
   int m;
   char shellcmd[BUFSIZ];
   char err_line[MAXCHAR];
   sscanf(buildline,
      "%c %[]a-z A-Z0-9,<.>\/?;:\\\'\"\[\{\}\|\`~=+_)(*&^%$#@!-]",
      &firstc, shellcmd);
   for(m=0; m< BUFSIZ; m++) {
      buildline[m] = '\0';
   }
   if( (fpshell = popen(shellcmd, "r")) != NULL) {
       fgets(buildline, MAXCHAR, fpshell);
       sscanf(buildline, "%s", linename);
   }
   else {
       sprintf(err_line, "assist: mscript: getfname: %s", shellcmd);
       perror(err_line);
       done1();
       exit(BADCMD);
   }
}

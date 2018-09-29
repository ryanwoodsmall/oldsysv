/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)setup:update.c	1.8"
#include "msetup.h"
extern char *getenv();
extern FILE *fpassistrc;  /* pointer to .assistrc file in user's home directory */
extern int newuser;
char assistrc[MAXCHAR];  /*complete pathname of user's .assistrc file*/

update(anystr)
char anystr[MAXCHAR];

{
      FILE *fptmprc;               /* pointer to temporary assistrc */
      char word1[NCHAR], wordrc[NCHAR];
      char tmpbuf[MAXCHAR];
      char tmpname[MAXCHAR];
      char err_line[MAXCHAR];
      int dstate = FALSE;
      int foundstate = FALSE;

      rewind(fpassistrc);

      sprintf(tmpname, "/tmp/%s.assistrc", getenv("LOGNAME") );

      if ( (fptmprc = fopen(tmpname, "w") ) == NULL) {
          sprintf(err_line,"assist: setup: %s", tmpname);
          perror(err_line);
          exit(1);
      }

      sscanf(anystr,"%s", word1);

      if (strcmp(word1, "DELETE") == 0) {
          dstate = TRUE;
          sscanf(anystr,"DELETE %s", word1);
      }

      while( foundstate == FALSE &&
                (fgets(tmpbuf,MAXCHAR, fpassistrc) != NULL)  ) {
          sscanf(tmpbuf, "%s", wordrc);
          if (strcmp(word1, wordrc)== 0) {
             if (dstate == FALSE) fputs(anystr, fptmprc);
             foundstate = TRUE;
          }
          else fputs(tmpbuf, fptmprc);
      }

      while (fgets(tmpbuf,MAXCHAR,fpassistrc) != NULL) fputs(tmpbuf, fptmprc);
      
      if (foundstate == FALSE && dstate == FALSE) fputs(anystr, fptmprc);

      fclose(fptmprc);
      
      sprintf(tmpbuf,"cp /tmp/%s.assistrc %s", getenv("LOGNAME"), assistrc);
      if ( access("/bin/cp", EXECUTE) != 0 || 
           access(assistrc, WRITE) != 0 ||
             (system(tmpbuf) != 0) ) { /* if the copy was no good */
         if (newuser == TRUE) unlink(assistrc);
         unlink(tmpname);
         fprintf(stderr, "assist: setup update: Cannot copy or write to .assistrc file\n");
         exit(1);
      }
      unlink(tmpname);
     
}

/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
# include <stdio.h>

# define NO 0
# define YES 1
# define TMPSIZE 10

extern int line;

int macro = NO, rflag = NO, opt = YES, rdonly = NO;
int zflag = 0;          /* .long for long | .double for double */
int errflag = NO;
int disp = 2;
char *ofile = (char *) 0;
char *ifile = (char *) 0;

main(argc,argv)
   int argc;
   char *argv[];
{
   int tfd, mp, status, fd[2];

   setflags(--argc,++argv);

   if ( ifile ) {
      if ( (tfd = open(ifile,0)) == -1 )
         error("Unable to open input file");
      replace(0, tfd);
      }

   if ( ofile ) {
      if ( (tfd = creat(ofile,0664)) == -1 )
         error("Unable to open output file");
      replace(1, tfd);
      }

   if ( macro ) {
      if ( pipe(fd) )
         error("Unable to run macro pre-processor");
      if (! (mp = fork()) ) { /* child process */
         close(fd[0]);
         replace(1,fd[1]);
         execl("/usr/bin/m4","m4",0);
         error("Unable to run macro pre-processor");
         }
      if (mp == -1)
         error("Unable to run macro pre-processor");
      close(fd[1]);
      replace(0,fd[0]);
      }

   aspass1();

   if ( macro )
      if (wait(&status) == -1)
         error("Unable to run macro pre-processor");

   if ( rflag && ifile )
      unlink(ifile);
   exit(errflag);
}

/* replace - replace either stdin, stdout or stderr with file descriptor n */
replace(std,n)
   int std, n;
{
   close(std);
   dup(n);
   close(n);
}

/* setflags - parse command line and establish environment */
setflags(ac,av)
   int ac;
   char *av[];
{
   char ch, *arg;
   int ap;

   for (ap = 0; ap < ac; ap++) {
      arg = av[ap];
      if (*arg == '-')
         while ( ch = *++arg )
            switch (ch) {
            case 'm':
               macro = YES;
               break;
            case 'o':
               if ( ofile ) {
                  fprintf(stderr,"multiple output files\n");
                  errflag = 1;
                  }
               ofile = av[++ap];
               break;
            case 'V':
               /* version flag */
               fprintf(stderr,"assembler - 2.2asg (5 dec 84)\n");
               break;
            case 'n':
               opt = NO;
               break;
            case 'R':
               rflag = YES;
               break;
            case 'r':
               rdonly = YES;
               break;
            case 'b':
               disp = 0;
               break;
            case 'w':
               disp = 1;
               break;
            case 'd':
               disp = 2;
               break;
            case 'z':
               zflag = 1;
               break;
            case '?':
               errflag = 1;
               break;
            default:
               fprintf(stderr,"unrecognized flag: %c\n", ch);
               errflag = 1;
               break;
            }
      else {
         if ( ifile ) {
            fprintf(stderr,"multiple input files\n");
            errflag = 1;
            }
         ifile = arg;
         }
      }

   if ( errflag ) {
      fprintf(stderr,"usage: as [-o objfile][-n][-m][-R][-r][-[bwd]][-V] file-name\n");
      exit(1);
      }

   if ( (! ofile) && ( ifile ) ) {
      char *ip, *op;

      ofile = (char *) allocate(20);
      for (op = ofile, ip = ifile; *op = *ip; ip++ )
         if (*ip == '/')
            op = ofile;
         else
            op++;
      if (op[-1] == 's' && op[-2] == '.')
         op[-2] = '\0';
      strcat(ofile,".o");
      }
}

error(s)
   char *s;
{
   fprintf(stderr,"as - %d: %s\n", line, s);
   exit(1);
}
warn(s)
   char *s;
{
   fprintf(stderr,"as - %d: %s\n", line, s);
}

int allocate(size)
   int size;
{
   int loc;

   if (loc = malloc(size))
      return loc;
   error("out of memory");
}

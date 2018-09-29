/*	@(#)sys.h	1.2	*/
/*
 *  External declarations for system call library routines
 */

extern int  close();
extern int  creat();
extern int  execl();
extern int  exit();
extern int  fork();
extern long lseek();
extern int  open();
extern int  ptrace();
extern int  read();
extern char *sbrk();
extern int  (* signal())();
extern int  wait();
extern int  write();

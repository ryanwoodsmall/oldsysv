/*	@(#)execargs.h	1.1	*/
#if vax
char **execargs = (char**)(0x7ffffffc);
#endif

#if pdp11
char **execargs = (char**)(-2);
#endif

#if u3b
/* empty till we can figure out what to do for the shell */
#endif

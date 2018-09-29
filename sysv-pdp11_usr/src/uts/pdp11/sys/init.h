/* @(#)init.h	1.3 */
extern int clkstart(),cinit(),binit(),errinit(),iinit(),ubminit();
#ifdef ST_0
extern int stinit();
#endif
#ifdef VPM_0
extern int vpminit();
#endif
#ifdef X25_0
extern int x25init();
#endif

/*	Array containing the addresses of the various initializing	*/
/*	routines executed by "main" at boot time.			*/

int (*init_tbl[])() = {
	clkstart,
	cinit,
	binit,
	errinit,
	ubminit,
	iinit,
#ifdef ST_0
	stinit,
#endif
#ifdef VPM_0
	vpminit,
#endif
#ifdef X25_0
	x25init,
#endif
	0
};

/* @(#)name.c	6.1 */
#include "sys/utsname.h"

struct utsname utsname = {
	SYS,
	NODE,
	REL,
	VER,
	MACH,
};

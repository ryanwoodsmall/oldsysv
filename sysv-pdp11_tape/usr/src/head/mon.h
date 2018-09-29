/*	@(#)mon.h	1.4	*/
struct hdr {
	char	*lpc;
	char	*hpc;
	int	nfns;
};

struct cnt {
	char	*fnpc;
	long	mcnt;
};

typedef unsigned short WORD;

#define MON_OUT	"mon.out"
#if pdp11
#define MPROGS0	300
#else
#define MPROGS0	600
#endif
#define MSCALE0	4
#define NULL	0

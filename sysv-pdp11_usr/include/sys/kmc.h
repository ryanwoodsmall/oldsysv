/* @(#)kmc.h	1.1 */
#define	KSTEP	1
#define	KMS	2
#define	KCSR	3
#define	KSTOP	4
#define	KMCLR	5
#define	KRUN	6
#define	KLU	7
#define	KWRCR	8
#define	KRESET	9

#define	KCSETA	(('k'<<8)|1)

struct kmcntl {
	int	kmd;
	short	*kcsr;
	int	kval;
};

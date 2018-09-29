/ @(#)mch.h	1.1
/ machine language assist
/ for 11/45 or 11/70 CPUs

/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
halt	= 0
wait	= 1
reset	= 5
rtt	= 6

PRSUPR	= 010000
PRUSER	= 030000
ALTREG	=  04000
HIPRI	= 340
HIGH	= 7

.globl	_u
_u	= 140000
usize	= 16.

CSW	= 177570
PS	= 177776
SSR0	= 177572
SSR3	= 172516
KISA0	= 172340
KISD0	= 172300
KDSA0	= 172360
KDSD0	= 172320
SISA0	= 172240
SISD0	= 172200
UISA0	= 177640
UISD0	= 177600
UDSA0	= 177660
UDSD0	= 177620
MSCR	= 177746
IO	= 177600
SL	= 177774
PIR	= 177772
UBMAP	= 170200

OVDEBUG = 1

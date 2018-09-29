/ @(#)mch.h	1.1
/ machine language assist
/ for 11/34, 11/23, 11/24, or 11/40 CPUs

/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst

halt	= 0
wait	= 1
reset	= 5
rtt	= 6

exec	= 11.

PRUSER	= 030000
HIPRI	= 340
NOCLK	= 40		/ don't process wakeup's in clock.c

U_PROCP	= 162.
P_FLAG	= 1.
SLOAD	= 1
P_ADDR	= 18.

usize	= 16.

PS	= 177776
SSR0	= 177572
KISA0	= 172340
KISD0	= 172300
UISA0	= 177640
UISD0	= 177600
IO	= 177600
PIR_VEC	= 240

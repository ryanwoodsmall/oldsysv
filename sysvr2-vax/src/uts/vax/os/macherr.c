/* @(#)macherr.c	6.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/sysinfo.h"

	struct memctl {
		int	mcra, mcrb, mcrc;
	};

char *macher[] = {
	"CP Read Timeout",
	"?",
	"CP Translation Buffer Parity Error",
	"CP Cache Parity Error",
	"?",
	"CP Read Data Substitute",
	"?",
	"?",
	"?",
	"?",
	"IB Translation Buffer Parity Error",
	"?",
	"IB Read Data Substitute",
	"IB Read Timeout",
	"?",
	"IB Cache Parity Error",

	"?",
	"Control Store Parity Error",
	"CP Translation Buffer Parity Error",
	"CP Cache Parity Error",
	"CP Read Timeout",
	"CP Read Data Substitute",
	"Microcode lost",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?"
};
macheck(type,ces,upc,vaddr,dr,tb0,tb1,paddr,par,sbi,pc,psl)
{
	printf("\nMachine check, type %x\n",type);
	if ((type&0xf0)==0)
		printf("%s Fault\n",macher[type&0xf]);
	else
		printf("%s Abort\n",macher[16+(type&0xf)]);
	printf("pc = %x, psl = %x\n", pc, psl);
	printf("virt/phys addr = %x/%x\n",vaddr,(paddr&0xfffffff)<<2);
	printf("ces = %x, sbi = %x, par = %x\n", ces, sbi, par);
	printf("tb0 = %x, tb1 = %x\n", tb0, tb1);
}

char *sbimsg[] = {
	"silo compare",
	"-",
	"alert",
	"fault",
	"CPU timeout"
};
#define	CRD	0x4000
sbierr(type, pc, ps, sbifs, sbisc, sbimt, sbier, sbita, silo)
{
	register i, *p;
	register n;
	extern struct memctl *memcvad;

	n = syserr.sbi[type]++;
	if (type == 1) {
		if (n < 10 || (n%20)==0) {
			printf("%s%d: ", sbier&CRD?"CRD":"RDS", n);
			printf("er %x, ma %x\n", sbier, memcvad->mcrc);
		}
		logmemory(sbier, memcvad->mcrc);
		memcvad->mcrc = 0x30000000;
		return;
	}
	printf("\nSBI %s, cnt %d, pc %x, ps %x\n", sbimsg[type], n, pc, ps);
	printf(" fs %x, er %x, ta %x, mt %x",
		sbifs, sbier, sbita, sbimt);
	if (sbisc)
		printf(", sc %x\n", sbisc);
	else
		printf("\n");
	if ((sbifs&0x00010000) || (sbisc&0x80000000)) {
		p = &silo;
		printf("Silo:");
		for (i=0; i<8; i++)
			printf(" %x", *p++);
		printf("\n");
		for (; i<16; i++)
			printf(" %x", *p++);
		printf("\n");
	}
}

char	*Cmacher[] = {
	"?",
	"CS Parity Error",
	"Memory Error",
	"Cache Parity Error",
	"Write Bus Error",
	"Corrected Data",
	"?",
	"Bad IRD"
};

machck(type, vaddr, oldpc, mdr, smr, rlto, tbgpr, caer, ber, mcesr, pc, psl)
{
	printf("\nMachine Check, type %x\n%s\n", type, Cmacher[type]);
	printf("pc= %x, psl = %x\n", pc, psl);
	printf("Virt addr = %x, error pc = %x, mdr = %x\n",
			vaddr, oldpc, mdr);
	printf("Mode = %x, read lock timeout = %x, tb = %x\n",
			smr, rlto, tbgpr);
	printf("Caer = %x, bus error = %x, mcesr = %x\n", caer, ber, mcesr);
	/* translation buffer parity error, continue */
	if ((type&0x0f) == 2)
		return 1;
	return 0;
}

cmierr(type, pc, psl)
{
	register n;
	extern struct memctl *memcvad;

	n = syserr.sbi[type]++;
	if (type == 0) {
		if (n < 10 || (n%20)==0) {
			printf("CRD%d: ", n);
			printf("mem csr0 = %x\n", memcvad->mcra);
		}
		logmemory(memcvad->mcra, (memcvad->mcra)&0x00fffe00);
		memcvad->mcra = memcvad->mcra;
		return;
	}
	printf("\nCMI Mem Write Timeout, cnt = %d, pc = %x, psl = %x\n",
			n, pc, psl);
}

char	*mill[5] = {
	"Unknown type",
	"Protection violation on on interrupt stack",
	"Invalid page store on interrupt stack",
	"Bad Main return",
	0
};
machill(type)
{

	printf("Machine software error: ");
	if (type < 0 || type >= 5)
		printf("Bad type\n");
	else
		printf("%s\n", mill[type]);
}

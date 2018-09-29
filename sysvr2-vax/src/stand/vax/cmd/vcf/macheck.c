/*	@(#)macheck.c	1.1	*/
extern int errflg;
char *macher[] = {
	"CP Read Timeout",
	"-",
	"CP Translation Buffer Parity Error",
	"CP Cache Parity Error",
	"-",
	"CP Read Data Substitute",
	"-",
	"-",
	"-",
	"-",
	"IB Translation Buffer Parity Error",
	"-",
	"IB Read Data Substitute",
	"IB Read Timeout",
	"-",
	"IB Cache Parity Error",
	"-",
	"Control Store Parity Error",
	"CP Translation Buffer Parity Error",
	"CP Cache Parity Error",
	"CP Read Timeout",
	"CP Read Data Substitute",
	"Microcode lost"
};
macheck(type,ces,upc,vaddr,dr,tb0,tb1,paddr,par,sbi,pc,psl)
{
	if(!errflg) return;
	printf("\nMachine check, type %x\n",type);
	if ((type&0xf0)==0)
		printf("%s Fault\n",macher[type&0xf]);
	else
		printf("%s Abort\n",macher[16+(type&0xf)]);
	printf("pc = %x, psl = %x\n",pc, psl);
	printf("virtual/physical addr = %x/%x\n",vaddr,paddr<<2);
	printf("ces = %x, sbi = %x\n",ces,sbi);
}

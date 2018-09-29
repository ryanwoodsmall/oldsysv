/*	@(#)cf.c	1.3	*/
	/* system configuration verification program */
/*
	This program runs in a stand-alone environment,
	or under unix with reduced capabilities.

	The program starts by loading configuration
	information from the designated unix
	executable program (default is /unix),
	and then sets up a system control block in low
	memory for interrupt handling.

	Next, each NEXUS (interface to the VAX processor) is
	tested for the existence of an adapter {see adinit()}.
	If one is found, its control register is read to
	determine the type of adapter present, and the following
	action is taken:

	1. If the adapter is for memory, ckmem() is called
	to determine the memory size, starting address of
	the memory, the chip size (4K or 16K) in the memory,
	and whether or not the memory is interleaved.

	2. If the adapter is a MASSBUSS Adapter (MBA),
	ckmbdev() is called to evaluate the type and number
	of devices present.  The devices presently known
	are Disks RP06, RM05, RP07, and tapes TE16,
	TU16, and TU78.  For each device located, ckmbdev()
	reports if the device in on-line, and if it is 
	dual ported.

	3. If the adapter is a UNIBUS Adapter (UBA),
	the UBA address space is reported, and ckubdev()
	is called to evaluate the UNIBUS devices.  This is
	done by scanning the information previously read
	from the unix executable, and calling ckdev() to access 
	each device listed.  For each device type in the 
	configuration tables, ckdev is called to
	verify that the named device is present. If so
	a specific test routine is called (ckdz(), ckkmc(), etc.),
        to exercise simple device functions and if
	appropriate, determine its interrupt vector(s)
	and interrupt priority.

	Background routines, many extracted from unix system
	code, are used primarily for handling of interrupts
	and traps generated during device location and testing.

/*  */
#include <a.out.h>
#undef	n_name
#include <fcntl.h>

#define NX_ADD0 0x20000000	/* phys addr NEXUS 0 */
#define MINAD	0
#define MAXAD	11
#define ADinit 01	/* Adapter init bit */
#define IFS 0x40	/* interrupt field switch */
#define BRIE 0x20	/* BR interrupt enable */
#define NX_APU 0x400000	/* Adapter Power Up */
#define NX_APD 0x800000	/* Adapter Power Down (but present) */
#define NX_UPD 0x20000	/* Unibus Power Down (but present) */
#define MB_ERR 0x3080	/* expected error bits in MBA status reg */
#define NED 0x10000	/* Non-existant device */
#define DV_GO 1		/* go bit */
#define DV_DC 010	/* drive clear */
#define DV_DRY 0x80	/* drive ready, status reg */
#define DV_DPR 0x100	/* device present, status reg */
#define DV_PGM 0x200	/* select sw programmable, status reg */
#define DV_DRQ 0x800	/* dual ported, status reg */
#define DV_ERR 0x4000	/* composite error, status reg */
#define DV_MOL 0x1000	/* medium-online bit in status reg */
#define BLKSIZ 512
#define MAXERR 50

#define	GDRP04	020	/* drive type values */
#define GDRP05	021
#define GDRP06	022
#define GDRP07	042
#define GDRM05	027
#define GDRM80	026
#define DTYPE	0777	/* drive type mask */

#define dvnam(X) nl[X].n_name
#define dvadd(X) nl[X].n_value
#define dvcnt(X) nl[X+1].n_value

/*
struct	nlist { * symbol table entry *
	char	n_name[8];	* symbol name *
	char	n_type;		* type flag *
	char	n_other;
	short	n_desc;
	unsigned n_value;	* value *
*/
struct nlist nl[] =
{
	{ "_umemvad"},
			/* the following must be in pairs
			   of 'xxx_add', and 'xxx_cnt' */
	{ "_dh_addr"},
	{ "_dh_cnt"},
	{ "_dn_addr"},
	{ "_dn_cnt"},
	{ "_dz_addr"},
	{ "_dz_cnt"},
	{ "_dza_add"},
	{ "_dza_cnt"},
	{ "_dzb_add"},
	{ "_dzb_cnt"},
	{ "_kmc_add"},
	{ "_kmc_cnt"},
	{ "_vpm_add"},
	{ "_vpm_cnt"},
	{ "_dm_addr"},
	{ "_dm_cnt"},
	{ "_cat_add"},
	{ "_cat_cnt"},
	{ "_lp_addr"},
	{ "_lp_cnt"},
	{ "_kl_addr"},
	{ "_kl_cnt"},
	{ "_vp_addr"},
	{ "_vp_cnt"},
	{ "_dmc_add"},
	{ "_dmc_cnt"},
	{ "_dmr_add"},
	{ "_dmr_cnt"},
	{ "_dmb_add"},
	{ "_dmb_cnt"},
	{ ""},
};
#define L_LIST sizeof nl/sizeof nl[0] 

/**/
extern cknul(),ckdh(),ckdn(),ckdz(),ckkmc();
struct {
	char *sysn, *name;
	int nareg;
	int nline;
	int (*ckdev)();
} devnam[] = {
	{ "____", "???",  1, 1 , cknul },
	{ "_cat", "cat",  1, 1 , cknul },
	{ "_dh_", "dh", 8, 16 , ckdh },
	{ "_dm_", "dm", 2, 1 , cknul },
	{ "_dmc", "dmc", 4, 1, cknul },
	{ "_dn_", "dn", 4, 4 , ckdn },
	{ "_dz_", "dz", 4, 8 , ckdz },
	{ "_dza", "dza", 4, 8 , ckdz },
	{ "_dzb", "dzb", 4, 8 , ckdz },
	{ "_dzc", "dzc", 4, 8 , ckdz },
	{ "_kmc", "kmc", 4, 1 , ckkmc },
	{ "_lp_", "lp", 2, 1 , cknul },
	{ "_vp_", "vp", 8, 1 , cknul },
	{ "_vpm", "vpm", 4, 1 , cknul },
};

#define MAXDEV 30
struct {
	char *name;
	unsigned int addr, rvec;
} ubdev[MAXDEV];	/* if you've got more than MAXDEV, */
			/* the last one is overwritten repetitively */ 
int	ubdvno = 0;

union NX_REGS {
	struct MEMR {
		int M_cra, M_crb, M_crc;
	} MEMr;
	struct MBAR {
		int M_csr, M_cr, M_sr,
		M_var, 	M_bc;
	}Mbar;
	struct UBAR {
		int U_cf, U_cr, U_sr, U_dcr,
		U_fmer, U_fuar, U_fmer2, U_fmar2;
	}Ubar;
	struct {
		char fill1[0x400];
		struct DVR {
			int	DV_cr, DV_ds, DV_er1, DV_mr;
			int	DV_as, DV_da, DV_dt, DV_la;
			int	DV_sn, DV_off, DV_cyl, DV_cc;
			int	DV_err2, DV_err3, DV_Epos, DV_Epat;
			int	fill2[16];
		} DVreg[8];
	}Unit;
	struct MAP {long fill3[0x200], M_map[256];}Map;
} *NXptr, *UBptr;
char *ibuf, *obuf;	/* buffer pointers */
extern char *malloc();
extern setrec(), mcheck();
/**/
#ifdef STANDALONE
extern	srtscb, endscb;
#endif
int errflg, intrflg = 1, fd;
unsigned data0, mem0;
unsigned ubspc = 0760000;
unsigned uba_sa = 0x20100000;

main(argc,argv) 
char	*argv[];
{
	int nexus, i, j;
	char *fp = "/unix";
	errflg = 0;

#ifdef VERS
	printf("\nVAX Configuration Verification\n\t %s",VERS);
#endif
	if(argc > 1) fp = argv[1];
	ldnl(fp);
	if((fd = open(fp,0)) < 0) printf("\n*** cannot open %s\n",fp);
#ifdef STANDALONE
	movscb();	/* put system control block in low memory */
	for(nexus = MINAD; nexus <= MAXAD; nexus++){
		errflg = 0;
		if(adinit(nexus)){
			errflg++;
			printf("\n\nNEXUS %d:",nexus);
			if((NXptr->Mbar.M_csr & 0xff) < 0x20){
				printf(" MEM;");
				ckmem();
			}
			else if((NXptr->Mbar.M_csr & 0xff) == 0x20){
				printf(" MBA;");
				ckmbdev();
			}
			else if((NXptr->Ubar.U_cf & 0xfc) == 0x28){
				uba_sa = 0x20100000 + (0xc0000 &
					(NXptr->Ubar.U_cf << 18));
				ubspc = (uba_sa & 0xc0000) + 0760000;
				printf(" UBA; address space: %x",uba_sa);
				UBptr = NXptr;
				ckubdev();
			}
			else printf(" Unknown device type %x",
				NXptr->Ubar.U_cf);
		}
	}
	printf("\n\nUNIBUS device summary\n");
	for(i = 0; i < ubdvno; i++){
		printf("\n\t   %5s %o",
			ubdev[i].name,ubdev[i].addr);
		if(ubdev[i].rvec > 1)
			printf(" %o",ubdev[i].rvec);
	}
#endif
#ifndef STANDALONE
	ckubdev();
#endif
	printf("\nEND\n\n");
}
 
/* */
#ifdef STANDALONE
	/*
	* Attempt to initialize an adapter on this NEXUS.
	* If no adapter is present, the resulting machine check
	* is fielded by Xmacheck(), which sets up a return
	* to erret(). Erret() returns a 0 to the caller of this
	* routine, indicating no adapter exists (or one
	* is present but inoperable).
	*/
adinit(nexus)
int nexus;
{
	register int page, *mp0, i;
	NXptr =(union NX_REGS *)(NX_ADD0 + (nexus*0x2000));
	if(NXptr->Mbar.M_csr & NX_APD){
		printf("\nAdapter powered down");
		return(0);
	}
	NXptr->Mbar.M_cr = ADinit;
	return(1);
}

/*
*	check devices on this MBA
*/
ckmbdev()
{
	int i,typ;
	struct DVR *dreg;

	for(i = 0; i < 8; i++){
		dreg = &NXptr->Unit.DVreg[i];
		if(typ = adtyp(dreg)){
			printf("\n   DEV %d -",i);
			switch(typ & DTYPE){
			case GDRP04:
				printf(" RP04");
				break;
			case GDRP05:
				printf(" RP05");
				break;
			case GDRP06:
				printf(" RP06");
				break;
			case GDRM05:
				printf(" RM05");
				break;
			case GDRM80:
				printf(" RM80");
				break;
			case GDRP07:
				printf(" RP07");
				break;
			default:
				if((typ & 0x4020) == 0x4020)
					printf(" TE16");
				else if((typ & 0x4010) == 0x4010)
					printf(" TU16");
				else if((typ & 0x4041) == 0x4041)
					printf(" TU78");
				else if(typ & 0x4000)
					printf(" Tape type %x",
						typ & 0x1ff);
				else printf(" type %x",typ);
			}
			if(dreg->DV_ds & DV_MOL) printf(" MOL");
			if(dreg->DV_dt & DV_DRQ) printf(" Dual Ported");
		}
		NXptr->Unit.DVreg[i].DV_cr = DV_DC | DV_GO;
	}
}

/*
*	try to determine adapter type from device regs
*/
adtyp(unit)
struct DVR *unit;
{
	int typ;

	typ = unit->DV_dt & 0x71ff;
	if(NXptr->Mbar.M_sr & NED){
		NXptr->Mbar.M_sr = NED; /* clears bit */
		return(0);
	}
	return(typ);
}

movscb()
{
	long	*vec, *scb;

	vec = 0;
	scb = &srtscb;
	while(scb < &endscb)
		*vec++ = *scb++;
}
#endif
erret()
{
	if(errflg) printf("*-*-*-*-*-*-*\n");
	return(0);
}
/**/
ckmem()
{
	int memsz,memsa,typ,ilv;

	typ = (NXptr->MEMr.M_cra >>3) &3;
	ilv = NXptr->MEMr.M_cra & 1;
	memsa = ((NXptr->MEMr.M_crb >> 15) & 0x1fff) * 64;
	memsz = ((NXptr->MEMr.M_cra >> 9) & 0x7f) * 64;
	printf(" size: %dK, start add: %dK",memsz,memsa);
	switch(typ){
		case 0: printf("\n*** NO ARRAY CARDS");
			break;
		case 1: printf(", 4K chips");
			break;
		case 2: printf(", 16K chips");
			break;
		case 3: printf("\n*** MIXED 4K & 16K CHIPS");
	}
	if(((typ == 1) || (typ == 2)) && ilv)
		printf(" interleaved");
}
/*
*	check devices on this UBA
*/
ckubdev()
{
	int i;
	unsigned ubadd; 
	struct exec {
		struct filehdr filehdr;
		struct aouthdr aouthdr;
		struct scnhdr scnhdr[3];
	} ux;

	if(fd < 0) return;
	if(lseek(fd,0,0) >= 0){
		if(read(fd,&ux,sizeof(ux)) > 0){
			printf("\n\ttext size: %x",ux.aouthdr.tsize);
			printf("\n\tdata size: %x",ux.aouthdr.dsize);
			printf("\n\tbss size: %x\n",ux.aouthdr.bsize);
		/* the location of the data segment in memory is */
			mem0 = (ux.aouthdr.tsize + 511) & ~511;
		/* the location of the data segment in a.out is */
			data0 = (ux.aouthdr.tsize + sizeof(ux));
		}
		else printf("*** read error *** (ckubdev())\n");
	}
	else printf("*** seek error *** (ckubdev())\n");
	for(i = 1; i < (L_LIST - 1); i += 2)
	if((ubadd = dvadd(i)) != 0){
#ifdef STANDALONE
		clruba();
#endif
		ckdev(dvnam(i),dvadd(i), dvcnt(i));
	}
	printf("\n\n");
}
clruba()
{
	int i;

	NXptr->Ubar.U_cr = ADinit;
	for(i=1;i<5000;i++);	/* we neeed a 500 u-sec delay */
	NXptr->Ubar.U_cr = IFS | BRIE;
}
ldnl(fp)
char *fp;
{
	int i;

	if(nlist(fp,nl)){
		printf("\n\nCannot access %s\n",fp);
		return(0);
	}
	printf("\n\nSystem: %s\n",fp);
	for(i = 0; i < L_LIST; i++)
		nl[i].n_value &= (long)0x7fffffff;
}
cknul(addr)
unsigned short *addr;
{
	printf(" (%x)",addr);
	return(1);
}
ckvecs(ref)
int ref;
{
	int i, flg;

	flg = 0;
	for(i = 0; i < ref; i++){
		if(ubdev[i].addr == ubdev[ref].addr
		  || ((ubdev[i].rvec == ubdev[ref].rvec)
		  && (ubdev[i].rvec > 0x80))){
			printf("\n\tAddress/vector conflict with %5s %o %o",
			ubdev[i].name,ubdev[i].addr,ubdev[i].rvec);
			flg++;
		}
	}
/*
	if(!flg){
		printf("\n\tCheck %5s %o",
			ubdev[i].name,ubdev[i].addr);
		if(ubdev[i].rvec > 0x80)
			printf(" %o",ubdev[i].rvec);
	}
*/
	if(ref >= MAXDEV) ubdvno--;
}
/*
*	print device info
*/
ckdev(sysnam,ubadd,cntadd)
unsigned ubadd,cntadd;
char sysnam[];
{
	int list[16],count,i,ref,ivec;

	ref = 0;
	for(i = 0; i < (sizeof(devnam)/sizeof(devnam[0])); i++)
		if(strncmp(sysnam,devnam[i].sysn,4) == 0){
			ref = i;
			break;
		}
	printf("\n%.6s \t",devnam[ref].name);

	if(lseek(fd,cntadd - mem0 + data0,0) > 0){
		if(read(fd,&count,4) > 0){
			count /= devnam[ref].nline;
			lseek(fd,ubadd - mem0 + data0,0);
			if(count > 16) count = 16;
			read(fd,list,4*count);
			for(i = 0; i < count; i++){
				ubdev[ubdvno].name = devnam[ref].name;
				ubdev[ubdvno].addr = ubadd = 
					(list[i] & 0x7fffffff)
					- nl[0].n_value | ubspc;
				printf("\n\t%d: %o",i,ubadd);
				errflg = 0;
				ivec = (*devnam[ref].ckdev)(ubadd+uba_sa);
				if(!ivec){
					printf(" ---NO DEVICE PRESENT---");
				}
				else{
					ubdev[ubdvno].rvec = ivec;
					ckvecs(ubdvno++);
				}
				errflg++;
			}
		}
		else{
			printf("\n*** read error - cntadd : %x",cntadd);
			printf("  mem0 : %x",mem0);
			printf("  data0 : %x\n",data0);
		}
	}
	else{
		printf("\n*** seek error - cntadd : %x",cntadd);
		printf("  mem0 : %x",mem0);
		printf("  data0 : %x\n",data0);
	}
}
wait(i)
int i;
{
	if(i < 0) return;
	while(i-- > 0);
}

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/mau.c	10.8"

/*
 * support functions for the WE 32106 Math Accelerator Unit
 */

#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/reg.h"
#include "sys/immu.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/cmn_err.h"
#include "sys/mau.h"
/*
 *  Called from main() based on the driver initialization list built
 *  by LBOOT during kernel self-configuration. Determines
 *  basic sanity of MAU.
 */
  
int 	zeros[3];

void
mauinit()
{
	extern int mau_present;
	extern mau_faulted();

	/* MAU cannot be on a 32a chip */
	if (!is32b())
		return;
	mau_present = 0;		/* will be set if we trap */
	u.u_caddrflt = (int)mau_faulted;
	asm("	SPOP	&0x4fff");	/* poke mau with a mau nop  */
	mau_present = 1;

	/* In case MAU fpregs start out containing a trapping NaN,  	*/
	/* zero all four floating point registers. Clear the asr	*/
	/* so proc[0] does not inherit asr and appear to be a mau user. */

	movt0(zeros[0]);		/* clear floating reg 0 */
	movt1(zeros[0]);		/* clear floating reg 1 */
	movt2(zeros[0]);		/* clear floating reg 2 */
	movt3(zeros[0]);		/* clear floating reg 3 */
	movta(zeros[0]);		/* clear ASR */
	u.u_caddrflt = 0;

	asm("mau_faulted:");		/* external mem fault lands you here */

	/* flag set and non-zero caddrflt indicates SPOP nop succeeded */
	/* but register initializations failed. Print message and      */
	/* reset flag.						       */

	if (mau_present && u.u_caddrflt) {
		cmn_err(CE_WARN,"mauinit: ERROR: %s\n\t\t%s",
		    "Initialization Failed",
		     "TO FIX: Run MAU Diagnostics, Call For Service.");
		mau_present = 0;
	}
	u.u_caddrflt = 0;			/* clear caddrflt */
}

/*
 * Called from exece() to reinitialize mau to default IEEE environment.
 * This will set rounding mode to "round to nearest", mask all 
 * exceptions,  clear all sticky bits.
 */

void
mau_setup()
{
	u.u_mau.asr = 0;		/* initialize ublock asr save area */
	movta(u.u_mau.asr);		/* clear asr	*/
}

/*
 * Called from context switch code and during fork procdup()
 * to save the mau hardware state in the ublock.
 * Must clear asr (after reading DR) to prevent exceptions in
 * kernel mode.
 */

void
mau_save()
{

	/* Only need to save context if process used mau during */
	/* last time slice as indicated by non-zero CSC bit   */

	movfa(u.u_mau.asr);			/* move asr to ublock */

	if (u.u_mau.asr & ASR_CSC) {

		/* set u_spop flag to indicate found a mau user.     */
		/* Clear context switch bit and save other registers */

		u.u_spop |= U_SPOP_MAU;
		u.u_mau.asr &= ~ASR_CSC;
		movfd(u.u_mau.dr[0]);		/* save DR */
		movta(zeros[0]);		/* clr ASR */
		movf0(u.u_mau.fpregs[0][0]);	/* save floating reg 0 */
		movf1(u.u_mau.fpregs[1][0]);	/* save floating reg 1 */
		movf2(u.u_mau.fpregs[2][0]);	/* save floating reg 2 */
		movf3(u.u_mau.fpregs[3][0]);	/* save floating ref 3 */
	}
}

/* 
 * Called from kernel pswtch() function to restore the 
 * mau state when a process is set up to run.
 */

void
mau_restore()
{
	/*
	 * The full mau state is restored only if a previous
	 * mau_save() has detected this process is a mau user.
	 * Otherwise only the asr is restored.
	 */
	if (u.u_spop & U_SPOP_MAU) {
		movta(zeros[0]);		/* clr ASR */
		movt0(u.u_mau.fpregs[0][0]);	/* restore floating reg 0 */
		movt1(u.u_mau.fpregs[1][0]);	/* restore floating reg 1 */
		movt2(u.u_mau.fpregs[2][0]);	/* restore floating reg 2 */
		movt3(u.u_mau.fpregs[3][0]);	/* restore floating reg 3 */
		movta(u.u_mau.asr);		/* restore ASR */
		movtd(u.u_mau.dr[0]);		/* restore DR */
	} else {
		movta(u.u_mau.asr);		/* restore ASR */
	}
}


/* Called from panic() to save contents of mau registers in pan_regs */

void
mau_panic(ptr)
struct mau_st * ptr;
{
	asm("	MOVW	0x0(%ap), %r0");
	asm("	SPOPWS	&0x23fc, 0x0(%r0)");	/* save ASR */
	asm("	SPOPWT	&0x53fe, 0x4(%r0)");	/* save DR */
	movta(zeros[0]);			/* clr ASR */
	asm("	SPOPWT	&0x1c7e, 0x10(%r0)");	/* save floating reg 0 */
	asm("	SPOPWT	&0x1cfe, 0x1c(%r0)");	/* save floating reg 1 */
	asm("	SPOPWT	&0x1d7e, 0x28(%r0)");	/* save floating reg 2 */
	asm("	SPOPWT	&0x1dfe, 0x34(%r0)");	/* save floating reg 3 */
}



struct drform {			/* format of DR exponent field */
	unsigned dr_unused:14;
	unsigned dr_sign:1;
	unsigned dr_exp:17;
};


/* Mau_fault() is called from exhan to handle an external memory
 * fault caused by the MAU.  Mau_fault() returns
 * SIGFPE if a signal should be send to the user or 0 if the
 * the instruction is to be reexecuted.
 */

int
mau_fault()
{
	asr_t asr;			/* temp location for MAU ASR */
	struct dr {			/* temp location for MAU DR */
		struct drform drval;	/*   -- exponent */
		unsigned word1;		/*   -- 1st word of fraction */
		unsigned word2;		/*   -- 2nd word of fraction */
	} dr;
	int exponent;

	/* Check to see that SPOP id is 0 (for MAU).  PC points to
	 * an SPOP -- the support processor id is 4 bytes from the
	 * SPOP in the instruction.
	 */
	if (*((char *) u.u_pcb.pc + 4) != MAU_ID)
		return(SIGILL);

	/* Get the ASR from the MAU and store it in asr 
	 */
	movfa(asr.word);

	/* Check for an unmasked exception.  An unmasked exception has
	 * occured if both the mask and sticky bits for a given
	 * exception type are set.
	 */
	if ((asr.bits.um && asr.bits.us) || (asr.bits.pm && asr.bits.pss) ||
	    (asr.bits.om && asr.bits.os) || (asr.bits.im && asr.bits.is) ||
	    (asr.bits.qm && asr.bits.qs))
	{
	
		/* Some unmasked exception occurred
		 */

		/* Must correct for known MAU chip bug if
	 	 *    -- underflow masked (underflow mask bit 0)
	 	 *    -- inexact not masked (inexact mask bit 1)
	 	 *    -- underflow occurred (underflow sticky bit 1)
	 	 *    -- inexact occurred (inexact sticky bit 1)
	 	 *    -- result in DR is non-normalized (J-bit 0)
	 	 */

		/* Check ASR first
		 */
		if (!asr.bits.um && asr.bits.pm && asr.bits.us &&
			asr.bits.pss)
		{
   	 
			/* ASR inidicates that bug may have occured --
			 * get DR and check result
			 */
			asm("	SPOPWT &0x53fe, 0x4(%fp)");

			/* Check the J-bit (bit 63 of DR).  If J-bit is
			 * 0, correct for the chip bug by adding 1 to the
			 * exponent and normalizing the result.
			 */
			if (!(dr.word1 & JBIT))
			{
   	 
				exponent = dr.drval.dr_exp;
				exponent++;
				/* nothing to normalize if fraction is 0
				 */
				if (dr.word1 != 0 || dr.word2 != 0)
				{


					/* normalize the result by shifting
					 * the fraction to the left and 
					 * decrementing the exponent until the
					 * J-bit is set
					 */
					while (!(dr.word1 & JBIT))
					{
						dr.word1 = dr.word1 << 1;
						if (dr.word2 & 0x80000000)
							dr.word1 |= 1;
			    			dr.word2 = dr.word2 << 1;
			    			exponent--;
					}
				}

				/* put the exponent back in dr and write
				 * the new result back into the DR
				 */
				dr.drval.dr_exp = exponent;
				asm("	SPOPRT	&0x637f, 0x4(%fp)");
			}
		}

		/* return SIGFPE to force user exception
		 */
		return(SIGFPE);
	}
	/* No unmasked exception occured, so fault must have been
	 * caused by a request for a diagnostic non-trapping NAN.
	 * This feature is not supported in this release, so turn
	 * off the non-trapping NAN control bit and let the user
	 * retry the instruction.
	 */
	asr.bits.ntnc = 0;

	/* Set the context switch control bit so the OS knows the
	 * MAU state has changed and write the value into the MAU
	 * ASR.
	 */
	asr.bits.csc = 1;
	movta(asr.word);

	/* return 0 to force the user to retry the instruction
	 */
	return(0);
}



#define	DSIZE	8	/* number of words in double word destination */
#define	TSIZE	12	/* number of words in triple word destination */
#define PGNUM	0xfffff800
#define	PGOFF	0x000007ff


/*
 * chkprob(val, addr, fault_addr) -- check for occurence of a fault
 * during probe of instruction.  Return 1 if fault occurred, 0 if
 * no fault.  Val is typically a value returned by fubyte or fuword.
 */

static int
chkprob(val, addr, fault_addr)
int val;
char *addr, **fault_addr;
{

	/* access violation occurred if val = -1 */  
	if (val == -1)
	{
		*fault_addr = (char *) NULL;
		return(1);
	}
	/* page fault occurred.  U.u_pgproc cleared by k_trap */
	else if (!u.u_pgproc)
	{
		/* store faulting address at fault_addr */
		*fault_addr = addr;
		return(1);
	}
	/* no fault -- return 0 */
	return(0);
}


/* mau_fltsel
 *	If a mau command is to be processed as part of the
 *	restartability workaround, it is flagged in this array.  All
 *	other commands are not processed.  Only mau operations that
 *	can have a memory source and a memory destination will be
 *	processed.  FTOI and FTOD are not processed because the result
 *	in the DR is invalid when mau_pfault() is called.
 */

static char mau_fltsel[] = {
	0,	/* 0x00 no command	*/
	0,	/* 0x01 no command	*/
	1,	/* 0x02 add		*/
	1,	/* 0x03 sub		*/
	1,	/* 0x04 div		*/
	1,	/* 0x05 rem		*/
	1,	/* 0x06 mul		*/
	1,	/* 0x07 move		*/
	0,	/* 0x08 rdasr		*/
	0,	/* 0x09 wrasr		*/
	0,	/* 0x0a cmp		*/
	0,	/* 0x0b cmpe		*/
	1,	/* 0x0c abs		*/
	1,	/* 0x0d sqrt		*/
	1,	/* 0x0e rtoi		*/
	0,	/* 0x0f ftoi		*/
	1,	/* 0x10 itof		*/
	1,	/* 0x11 dtof		*/
	0,	/* 0x12 ftod		*/
	0,	/* 0x13 nop		*/
	0,	/* 0x14 erof		*/
	0,	/* 0x15 no command	*/
	0,	/* 0x16 no command	*/
	1,	/* 0x17 neg		*/
	0,	/* 0x18 ldr		*/
	0,	/* 0x19 no command	*/
	0,	/* 0x1a cmps		*/
	0,	/* 0x1b cmpes		*/
	0,	/* 0x1c no command	*/
	0,	/* 0x1d no command	*/
	0,	/* 0x1e no command	*/
	0};	/* 0x1f no command	*/


/*
 * mau_pfault(probe, fault_addr) -- complete potentially non-restartable
 *	MAU operation.
 *	If probe == 1 or 2, the routine probes the instruction pointed to
 *	by the pc to see if the following conditions are true:
 *		-- pc points to SPOPD2 or SPOPT2 instruction
 *		-- source and destination operands overlap
 *		-- destination crosses a page boundary
 *		-- all pages needed to decode and execute the
 *		   instruction are mapped and resident except the
 *		   page that contains the last portion of the
 *		   destination.
 *	If these conditions are met and probe == 1, the MAU operation is
 *	completed as described below.  If the conditions are met and probe == 2,
 *	the operation is completed later by a call to mau_pfault with
 *	probe == 3, which only writes the result.  If not, the routine returns
 *	and allows the user to retry the operation.  If a page is faulted in
 *	during the probe, the faulting address is returned in
 *	*fault_addr.
 *
 *	If probe == 0, the following conditions are assumed to be
 *	true:
 *		-- the routine is called only after page not present
 *		   or copy on write fault
 *		-- the faulting address is in *fault_addr
 *		-- the last access was a support processor write access
 *		-- the user's pc points to a SPOPD2 or SPOPT2 opcode
 *		-- a result is available in the MAU's DR (except for
 *		   FTOD or FTOI operations, which are not processed).
 *
 *	After completing the store of the result at the faulting
 *	address, mau_pfault() will update the user pc to point to
 *	the instruction after the SPOP that faulted.  If any errors
 *	are detected (access violations, invalid instruction, etc.),
 *	this routine returns without updating the user pc so that the
 *	user is forced to retry the instruction.
 */

/* NOTE:  probe can have one of three values:
 *		-- 0:  MAU_NOPROBE
 *		-- 1:  MAU_PROBESF
 *		-- 2:  MAU_PROBESB
 *		-- 3:  MAU_WRONLY
 */




void
mau_pfault(probe, fault_addr, fix)
int probe, *fix;
char **fault_addr;		/* used to contain/return fault addr */
{
	register char *pc;	/* faulting pc */
	register char *src_addr;
	register char *dest_addr;
	register char *taddr;
	cr_t cr;		/* buffer for MAU command word */
	asr_t asr;		/* temp location for MAU ASR */
	uint opsize;		/* the number of bytes to write */
	int pbyte;		/* temp for pc fetchs */
	pde_t *pdp;
	unsigned long drbuf[3];
	extern pde_t *vatopde();
	extern sde_t *vatosdt();
	extern char *nextop();
	extern int get_word();
	extern int chkprob();
	extern char *get_mau_oper();
	

	/* clear out probe flag in ublock */
	u.u_pgproc = 0;
	/* get pc for instruction that faulted */
	pc = (char *) u.u_pcb.pc;
	if (probe)
	{
		/* if probe == MAU_WRONLY, a previous probe has determined
		 * that the operation needs to be completed:  *fault_addr
		 * contains fault addr and *fix contains number of bytes to
		 * write */
		if (probe == MAU_WRONLY)
		{
			/* number of bytes to write is in *fix */
			opsize = *fix;
			/* set pc to point to second operand descriptor */
			pc = pc + 5;
			pc = nextop(fubyte(pc), pc);
			goto maucw;
		}
		*fault_addr = NULL;
		/* set flag in ublock -- if cleared later (by k_trap),
		 * a successful page fault has occurred */
		u.u_pgproc = 1;
		*fix = 0;
	}
	else
	{
		/* if not a probe and fault addr is not on a pg
		 * boundary -- no problem, just return */
		if (((uint) *fault_addr) & PGOFF)
			return;
	}
	/* get opcode -- is instruction mapped and present? */
	pbyte = fubyte(pc);
	if (probe && chkprob(pbyte, pc, fault_addr))
		return;
	/* does pc point to "MAU_SPECIAL"? */
	if (pbyte == SPOPD2)
		opsize = DSIZE;
	else if (pbyte == SPOPT2)
		opsize = TSIZE;
	else
		return;
	pc++;
	if (probe)
	{
		/* check for exception generated by MAU */
		movfa(asr.word);
		if (asr.bits.ecp)
			return;
	}
	/* get mau command word -- is it mapped and present? */
	if (get_word(probe, &cr.word, pc, fault_addr) == 0)
		return;
	/* check that we are processing a MAU spop (id = 0) and that
	 * the MAU opcode is one that we are able to handle */
	if (cr.mau.id != MAU_ID || mau_fltsel[cr.mau.opcode] == 0)
		return;
	pc += 4;
	if (probe)
	{
		/* get source operand address */
		if ((src_addr = get_mau_oper(probe, pc, fault_addr)) == NULL)
			return;
		/* word at source addr mapped and present? */
		if (chkprob(fubyte(src_addr), src_addr, fault_addr))
			return;
		/*  does src cross a page boundary? */
		taddr = (char *) ((uint) src_addr + (opsize - 4));
		if ((((uint) src_addr) & PGNUM) != (((uint) taddr) & PGNUM))
		{
			/* if yes, is second part mapped and present? */
			if (chkprob(fubyte(taddr), taddr, fault_addr))
				return;
		}
	}
	/* update pc past source operand descriptor -- note:  on probe,
	 * this byte has already been checked above */
	pbyte = fubyte(pc);
	pc = nextop(pbyte, pc);
	/* get destination address */
	if ((dest_addr = get_mau_oper(probe, pc, fault_addr)) == NULL)
		return;
	if (probe)
	{
		/* is destination mapped present? */
		if (chkprob(fubyte(dest_addr), dest_addr, fault_addr))
			return;
		/* check to see if src and destination overlap */
		if (!((src_addr <= dest_addr && dest_addr < (char *) ((uint) src_addr + opsize))
		    || (dest_addr <= src_addr && src_addr < (char *) ((uint) dest_addr + opsize))))
			return; 
		/* is first page of destination copy on write page? */
		pdp = svtopde(dest_addr);
		if (pdp->pgm.pg_cw)
			return;
		/* does dest cross page boundary?  If not, return. */
		taddr = (char *) (((uint) dest_addr + (opsize - 4)) & PGNUM);
		if ((((uint) dest_addr) & PGNUM) == (uint) taddr)
			return;
		/* last page mapped and present */
		pbyte = fubyte(taddr);
		if (pbyte == -1)
		{
			/* if probing during a stack bounds fault,
			 * the last page of detination may not be
			 * mapped yet -- so fubyte may have failed.
			 * Check to see if current probe address
			 * is between stack upper bound and sp -- 
			 * this is the area that is not mapped */
			if ((probe == MAU_PROBESB) &&
			    ((uint) taddr >= (uint) u.u_pcb.sub) &&
			    ((uint) taddr < (uint) u.u_pcb.sp))  
			{
				*fault_addr = taddr;
				*fix = opsize - ((uint) *fault_addr - (uint) dest_addr);
			}
			return;
		}
		else if (u.u_pgproc)
		{
			/* last page of dest present, copy on write page? */
			pdp = svtopde(taddr);
			if (!pdp->pgm.pg_cw)
				return;
		}
		*fault_addr = taddr;
	}
	/* if not probe and fault occurred on first word of dest, return */
	else if (dest_addr == *fault_addr)
		return;

	/* determine number of bytes to write */
	opsize = opsize - ((uint) *fault_addr - (uint) dest_addr);

maucw:
	/* read the contents of the DR into the local buffer drbuf */
	movfd(drbuf[0]);

	/* if opsize is not equal to either 4 or 8 (bytes), the
	 * destination address is invalid -- do not complete the
	 * operation if this is the case.  This is a sanity check
	 * of the destination address to make sure that the 
	 * destination and the faulting addresses are close, and
	 * therefore likely to be correct.  There are still several
	 * situations when this check does not guarantee that
	 * the correct number of words are written, but they
	 * occur only if the instruction has been overwritten
	 * during the execution of the SPOP.
	 */
	if (opsize == 4)
	{
		/* size is 4 bytes -- use suword to copy third
		 * word of the result to fault_addr */
		if (suword(*fault_addr, drbuf[2]) != -1)
			/* if copy successful, complete recovery by
			/* updating user's pc */
			u.u_pcb.regsave[K_PC] = (int) nextop(fubyte(pc), pc);
	}
	else if (opsize == 8)
	{
		/* size is 8 bytes -- use copyout to copy second
		 * and third words of the result to fault_addr */
		if (copyout(&drbuf[1], *fault_addr, 8) != -1)
			/* if copy successful, complete recovery by
			 * updating user's pc */
			u.u_pcb.regsave[K_PC] = (int) nextop(fubyte(pc), pc);
	}
}




/* two arrays giving operand descriptor size based on the high nibble's
 * value -- two, since if the register is the PC things change.
 * A 0 entry indicates that the addressing mode is illegal.
 */
static char
op_size[16] = { 
	0, 0, 0, 0, 0, 1, 1, 1, 5, 5, 3, 3, 2, 2, 0, 0 };
static char
pc_op_size[16] = { 
	0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 3, 3, 2, 2, 5, 0 };

static char *
nextop(reg, pc)
register int reg;
char *pc;
{
	register int	mode;

	mode = reg >> 4;
	reg = reg & 0xf;
	if (reg == 0xf)			/* register == PC*/
		return( pc + pc_op_size[ mode ] );
	else	/* everything else */
		return( pc + op_size[ mode ] );
}


/* 
 *	decode the operand descriptor starting at *pc,
 *	and return the address of the operand.
 * RETURN:
 *	0 if the descriptor is not valid for an SPOP instruction
 *	0 if the address is not word aligned
 *	operand address, otherwise.
 */

/* a Macro to return 0 (FALSE) if its argument is not word aligned,
 * otherwise, it returns its argument.
 */
#define AL(x)	( (((uint)(x) & 0x03) != 0) ? (char *)0 : (char *)(x) )
static char reg_trans[] = { K_R0, K_R1, K_R2, K_R3, K_R4, K_R5, K_R6,
		K_R7, K_R8, K_FP, K_AP, K_PS, K_SP, K_PC, K_PC, K_PC };

#define REG(x)	( u.u_pcb.regsave[reg_trans[x]])


static char *
get_mau_oper(probe, pc, fault_addr)
int	probe;
char	*pc;    /* pointer to 1st byte of descriptor */
char	**fault_addr;
{
	register int	reg;
	register int	mode;
	register int	pval;
	char *retval, *taddr;
	extern short get_half();

	pval = fubyte(pc);
	if (probe && chkprob(pval, pc, fault_addr))
		return(NULL);
	reg = pval & 0x0f;
	mode = pval >> 4;
	switch ( mode ) { /* high nibble contains address mode */

	default:	/* SHOULD NOT HAPPEN!	*/
	case 0x00:	/* literal mode		*/
	case 0x01:	/* literal mode		*/
	case 0x02:	/* literal mode		*/
	case 0x03:	/* literal mode		*/
	case 0x04:	/* register mode	*/
		/* 0x4f is word immediate but is still illegal */
	case 0x0f:	/* literal mode		*/
		return(NULL);

	case 0x05:	/* register deferred	*/
		if (reg == 0x0f) /* becomes halfword immediate mode */
			return(NULL);
		retval = (char *)(REG(reg));
		break;

	case 0x06:	/* FP short offset	*/
		if (reg == 0x0f) /* becomes byte immediate mode	*/
			return(NULL);
		retval = (char *)(u.u_pcb.regsave[K_FP] + reg);
		break;

	case 0x07:	/* AP short offset	*/
		if (reg == 0x0f) { /* becomes absolute mode	*/
			if (get_word(probe, &retval, pc + 1, fault_addr) == 0)
				return(NULL);
		} else {
			retval = (char *)(u.u_pcb.regsave[K_AP] + reg);
		}
		break;

	case 0x08:	/* word displacement	*/
	case 0x09:	/* word displacement deferred */
		if (get_word(probe, &retval, pc + 1, fault_addr) == 0)
			return(NULL);
		retval = (char *) (REG(reg) + (int) retval);
		if (mode == 0x09) {
			if (!AL(retval))
				return(NULL);
			taddr = retval;
			retval = (char *) fuword(taddr);
			if (probe && chkprob(retval, taddr, fault_addr))
				return(NULL);
		}
		break;

	case 0x0a:	/* halfword displacement */
	case 0x0b:	/* halfword displacement deferred */
		if (get_half(probe, &retval, pc + 1, fault_addr) == 0)
			return(NULL);
		retval = (char *) (REG(reg) + (int) retval);
		if (mode == 0x0b) {
			if (!AL(retval))
				return(NULL);
			taddr = retval;
			retval = (char *) fuword(taddr);
			if (probe && chkprob(retval, taddr, fault_addr))
				return(NULL);
		}
		break;

	case 0x0c:	/* byte displacement */
	case 0x0d:	/* byte displacement deferred */
		/* need to sign extend the next byte */
		pval = fubyte(pc + 1);
		if (probe && chkprob(pval, pc + 1, fault_addr))
			return(NULL);
		if ( pval & 0x80 )
			retval = (char *)( pval | 0xffffff00 );
		else
			retval = (char *)pval;
		retval = (char *) (REG(reg) + (int) retval);
		if ( mode == 0x0d ) {
			if (!AL(retval))
				return(NULL);
			taddr = retval;
			retval = (char *) fuword(taddr);
			if (probe && chkprob(retval, taddr, fault_addr))
				return(NULL);
		}
		break;

	case 0x0e:	/* expanded operand */
		if ( reg != 0x0f ) /* expanded mode is illegal */
			return(NULL);
		/* but if the register is the PC this is absolute deferred */
		if (get_word(probe, &retval, pc + 1, fault_addr) == 0)
			return(NULL);
		if (!AL(retval))
			return(NULL);
		taddr = retval;
		retval = (char *) fuword(taddr);
		if (probe && chkprob(retval, taddr, fault_addr))
			return(NULL);
		break;
	} /* end switch */

	return(AL(retval));
	
}

static int
get_word(probe, val, pc, fault_addr)
int probe, *val;
char *pc, **fault_addr;
{
	union {
		int	w;
		char	c[4];
	} tmp;
	int i;
	register int tval;

	for (i = 3; i >= 0; i--)
	{
		tval = fubyte(pc);
		if (probe && chkprob(tval, pc, fault_addr))
			return(0);
		tmp.c[i] = tval;
		pc++;
	}
	*val = tmp.w;
	return(1);
}

static short
get_half(probe, val, pc, fault_addr)
int probe, *val;
char *pc, **fault_addr;
{
	union {
		short	s;
		char	c[2];
	} tmp;
	int i;
	register int tval;

	for (i = 1; i >= 0; i--)
	{
		tval = fubyte(pc);
		if (probe && chkprob(tval, pc, fault_addr))
			return(0);
		tmp.c[i] = tval;
		pc++;
	}
	*val = tmp.s;
	return(1);
}

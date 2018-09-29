/*	@(#)machine.h	1.3	*/
/* 
 * VAX UNIX parameters 
 */

/* User block in core file */

#define ADDR_U ((unsigned) 0x7FFFF800)
#define ADRREG ((unsigned) 0x7B8)
union  Uuniontag {
    struct user strpart;
    char chrpart[ctob(USIZE)];
};

extern union Uuniontag Uunion;

/*  Note - the following define must appear after the above
 *  definition else the preprocessor will use the wrong u
 *  in the expansion of USIZE.
*/
#define u Uunion.strpart

/*
 *  Core layout for vax adb under unix 4.0
 *
 *                                TOP
 *             ---------------------------------------------
 *             |                                           |
 *             |               System Space                |
 *             |                                           |
 * 0x8000 0000 |-------------------------------------------|       -------
 * 0x7FFF FFFF |  (+0x7b8) registers (<- ksp)              |          ^
 *             |                  ^   System               |          |
 *             |            ar0 --|   Space                |          |
 *             |            pcb    (U-BLOCK)               |          |
 *             |-------------------------------------------| /e2      |
 * 0x7FFF F800 |               USP (core)                  |      u.u-ssize
 *             |                   |                       |          |
 *             |                   |                       |          |
 *             |                   v                       |          |
 *             |             (extended via                 |          |
 *             |             function call)                |          v
 *             |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| /b2   -------
 *             |                                           |
 *             |               NOT AVAILABLE               |
 *             |                                           |
 *             |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| /e1
 *             |                    ^                      |
 *             |      .bss + brk    | (extended via brk)   |
 *             |    a.out and core  | (extended via brk)   |
 *             |                    |                      |
 *             |-------------------------------------------| ?e2
 *             |                                           |
 *             |               DATA SPACE                  |
 *             |             a.out and core                |
 *             |                 .data                     |
 *             |                                           |
 * ((etext+511)|-------------------------------------------| ?b2 == /b1
 *  /512)*512  |                  padding                  |
 *      etext  |-------------------------------------------| ?e1
 *             |                                           |
 *             |                   TEXT                    |
 *             |                  (a.out)                  |
 *             |                                           |
 * 0x0000 0000 --------------------------------------------- ?b1
 *                                 BOTTOM
 */

/*
					Layout of a Core File

File Address    			    	  Source (Memory Address)
		-------------------------------
       0        >                             |            &u
		|    User block at time of    |
                |          core dump          |
                |                             |
                |-----------------------------|
  ctob(USIZE)   >                             | b1    ctob(u.u_tsize)
                |      User Data Segment      |
                |                             | e1
                |-----------------------------|
 ctob(USIZE) +  >                             | b2  0x80000000 - ctob(u.u_ssize)
ctob(u.u_dsize) |     User Stack Segment      |
                |                             |
                |-----------------------------| e2
ctob(USIZE) +   >
ctob(u.u_dsize) +
ctob(u.u_ssize - USIZE)
*/

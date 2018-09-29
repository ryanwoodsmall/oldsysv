/*     @(#)system.h	2.5     */
#define USEREGIONS      0       /* allows allocation by regions */
#define TRVEC   0               /* allows transfer vectors      */
#define UNIX    1               /* ld used as standard UNIX tool*/
#define DMERT   0               /* ld used to generate DMERT obj*/
#define TS      1               /* ld executed under UNIX-TS    */
#define RT      0               /* ld executed under UNIX-RT    */
#define B16     0               /* ld generates basic-16 object */
#define X86     0               /* ld generates extended 8086   */
#define N3B     0               /* ld generates 3B-20 object    */
#define M32     0               /* ld generates BELLMAC-32 obj  */
#define U3B     0               /* ld generates 3B-20Simplex obj*/
#define VAX     1               /* ld generates VAX 11/780 obj  */
#define DEC     0               /* ld generates DEC object      */
#define DEBUG   0               /* allow debugging flag         */
#define IANDD   0               /* allows separate i and d      */
#define SWABFMT 0               /* text in reverse order        */
#define ONEPROC 1               /* generate one process ld command */
#define NOSDP   1               /* no software demand paging */
#define PORTAR  1               /* ld uses UNIX 5.0 portable archive format */
#define PAGING	1		/* ld uses UNIX ?.0 demand paging */
#define COMMON  1		/* allow .comm symbols */

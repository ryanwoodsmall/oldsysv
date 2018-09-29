static char termodes__[]="@(#)termodes.h	1.1";
/* <: t-5 d :> */
#define RAWMODE      ioctl(1,TCSETA,&tm.raw)
#define COOKMODE     ioctl(1,TCSETA,&tm.cook)
#define ERASE       printf("\033\014")  /*  ESC NP  */
#define INQUIRE     printf("\033\005")  /*  ESC ENQ  */
#define ALPHAMODE   printf("\037")      /*  US  */
#define GINMODE     printf("\033\032")  /*  ESC SUB  */
#define GRAPHMODE   printf("\035")      /*  GS  */

#define SMALLFONT	printf("\033;")
#define GIN 6
#define GRAPH 7

static char gedstructs__[]="@(#)gedstructs.h	1.1";
#include <termio.h>
struct control{
	short change; /* TRUE when display buffer has been altered */
	short textf; /* TRUE when text is to be drawn */
	double wratio; /* window ratio: univ units per screen units */
	struct area w;  /*  window low x,y and high x,y  */
};
struct tekterm{
	int curls,curlw,curmode;
	struct termio cook;
	struct termio raw;
	int alphax,alphay;
};

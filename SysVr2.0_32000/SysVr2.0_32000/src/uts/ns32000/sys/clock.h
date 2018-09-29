/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 * @(#)timer.h	3.3	7/14/83
 * @(#)Copyright (C) 1983 by National Semiconductor Corp.
 */

/*************************************************************************
**									**
**		The 8253 programmable interval timer			**
**									**
**	Timers 0 & 1 are dedicated to the 8251 as the receive & xmit	**
**	baud clocks. Timer 2 is used as the system time clock.		**
**************************************************************************/

struct n8253 {			/* National 8253 device		*/
	unsigned char	rxclk;		/* clock 0 is receiver clock	*/
	char	fill0;
	unsigned char txclk;		/* clock 1 is xmitter clock	*/
	char	fill1;
	unsigned char clock;		/* clock 2 is system clock	*/
	char	fill2;
	unsigned char mode;		/* timer mode register		*/
	char	fill3;
};

/* Definition of mode control bits */

/* Counter selection */

#define	SC0	0x00			/* Select counter 0 */
#define SC1	0x40			/* Select counter 1 */
#define SC2	0x80			/* Select counter 2 */

/* Read/Load bits */

#define	LATCH	0x00			/* Latch counter		*/
#define	HIBYTE	0x20			/* Read/Load counter hi byte	*/
#define LOBYTE	0x10			/* Read/Load counter lo byte	*/
#define BYTES2	0x30			/* Read/Load both counter bytes	*/

/* Mode control bits */

#define TIM_MD0	0x00		/* Interrupt on terminal count	*/
#define TIM_MD1	0x02		/* Programmable one-shot	*/
#define TIM_MD2	0x04		/* Rate generator		*/
#define TIM_MD3	0x06		/* Square wave rate generator	*/
#define TIM_MD4	0x08		/* Software triggered strobe	*/
#define TIM_MD5	0x0a		/* Hardware triggered strobe	*/

/* BCD counter */

#define	BCDCOUNT 0x01			/* Count value is BCD */

/*
**	Counter 2 definitions for system time clock.			**
**									**
**	Input for the 8253 will be 1.8432MHZ. This will require a       **
**	value of 30720 to give a 60 HZ clock			        **/

#define C2MODE	SC2+BYTES2+TIM_MD2
#define C2VAL   30720

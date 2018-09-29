/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)help:switch.h	1.3"
/* define for the SWITCH construct for selecting among character strings */
#define CmP(s)			!strcmp(s_w_i_t_c_h,s)
#define NCmP(s)			!strncmp(s_w_i_t_c_h,s,sizeof(s)-1)

#define SWITCH(s)		{char *s_w_i_t_c_h; s_w_i_t_c_h = s; if(0){
#define CASE(s1)		}else if(CmP(s1)){
#define CASE2(s1,s2)		}else if(CmP(s1)||CmP(s2)){
#define CASE3(s1,s2,s3)		}else if(CmP(s1)||CmP(s2)||CmP(s3)){
#define CASE4(s1,s2,s3,s4)	}else if(CmP(s1)||CmP(s2)||CmP(s3)||CmP(s4)){
#define CASE5(s1,s2,s3,s4,s5)	}else if(CmP(s1)||CmP(s2)||CmP(s3)||CmP(s4)||CmP(s5)){
#define PCASE(s1)		}else if(NCmP(s1)){
#define PCASE2(s1,s2)		}else if(NCmP(s1)||NCmP(s2)){
#define PCASE3(s1,s2,s3)	}else if(NCmP(s1)||NCmP(s2)||NCmP(s3)){
#define PCASE4(s1,s2,s3,s4)	}else if(NCmP(s1)||NCmP(s2)||NCmP(s3)||NCmP(s4)){
#define PCASE5(s1,s2,s3,s4,s5)	}else if(NCmP(s1)||NCmP(s2)||NCmP(s3)||NCmP(s4)||NCmP(s5)){
#define DEFAULT			}else{
#define ENDSW			}}

static char SCCSID[]="@(#)treebldr.c	1.2";
/* <: t-5 d :> */
#include <stdio.h>
extern char *nodename;
#include "vtoc.h"
extern struct node *nd[];
extern int lastnode;

treebldr(fpi) 
FILE *fpi; {
  	int n,tmp_int;
  	char *ptr,buf[MAXLENGTH],*tokens[4];
	char *calloc();
/*
 *  treebldr creates a tree structure from an input data
 *  file of hierarchially related entries.
 */
	for	(n=0; n < MAXNODE && getentry(fpi,buf,tokens) != EOF; n++){
		if ((nd[n]=(struct node *) calloc(1,sizeof(struct node))) == NULL){
			lastnode=n-2;
			return(FAIL);
		}
		if (validid(ptr=tokens[ID])){
			if ((nd[n]->id=calloc(1,strlen(ptr)+1)) == NULL){
				lastnode=n-2;
				return(FAIL);
			}
			else	{
				nd[n]->level=levcal(ptr);
				strcpy(nd[n]->id,ptr);
			}
		}
		else	{
			ERRPR1(Invalid id: entry \"%s\"\, skipped.,ptr);
			n--;
			continue;
		}
		setwtst(tokens[LS],&nd[n]->weight,&nd[n]->style);
		ptr=tokens[TEXT];
		if ((nd[n]->text=calloc(1,strlen(ptr)+1)) == NULL){
			lastnode=n-2;
			return(FAIL);
		}
		else	strcpy(nd[n]->text,ptr);
		if((ptr=tokens[MARK])!=NULL){
			if ((nd[n]->mark=calloc(1,strlen(ptr)+1)) == NULL){
				lastnode=n-2;
				return(FAIL);
			}
			else	strcpy(nd[n]->mark,ptr);
		}
		else {
			if ((nd[n]->mark=calloc(1,1)) == NULL){
				lastnode=n-2;
				return(FAIL);
			}
			else	nd[n]->mark = "\0";
		}
  	}
	lastnode = --n;
	return(SUCCESS);
}

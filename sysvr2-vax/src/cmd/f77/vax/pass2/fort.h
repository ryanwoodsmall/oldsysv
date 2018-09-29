/*	@(#)fort.h	1.2	*/
/*	machine dependent file  */

label( n ){
	printf( ".L%d:\n", n );
	}

tlabel(){
	cerror("code generator asked to make label via tlabel\n");
	}

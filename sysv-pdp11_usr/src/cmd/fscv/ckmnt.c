/* @(#)ckmnt.c	1.1 */
ckmnt(fn)
char fn[];
{
	/*
			 * This subroutine trys to verify the file name passed in "fn" is
			 * on a raw device that does not have a mounted file system on it.
			 * Regular files are ignored for testing purposes.
			 */

	/*
			 * The following buffer is used to convert the raw device file
			 * name (character special) to a block special for ustat.
			 */

	char blknm[100];

	int rc;
	char ans[20];

#include <sys/types.h>
#include <sys/stat.h>
#include <ustat.h>

	struct stat st;
	struct ustat ust;

	/*
			 * If regular file just return
			 */
	rc = stat(fn,&st);
	if (rc != 0) {
		perror("stat error");
		exit(1);
	}
	if((st.st_mode&S_IFMT)==S_IFREG) return(0);
	/*
				 * If the file is a raw device try to convert it to a block special
				 * to see if it contains a mounted file system.  e.g.; convert /dev/rrp1
				 * to /dev/rp1 and ustat.
				 */
	if((st.st_mode&S_IFMT)==S_IFCHR) {

		/*
							 * See if the fn starts with the characters "/dev/r"
							 */
		if(strncmp(fn,"/dev/r",6) != 0) {
			do {
				printf("Unknown character special device: %s\n",fn);
				printf("Device must not contain a Mounted File System\n");
				printf("Do you wish to continue?  (y/n): ");
				scanf("%s",ans); 
			} 
			while ((ans[0] != 'y')&&(ans[0] != 'n'));
			if (ans[0] == 'n') exit(1);
			sync();
			return(0);
		}
		/*
							 * Character special of form /dev/r.  Remove the r and do stat to get
							 * real device info.
							 */
		strcpy(blknm,"/dev/");
		strcat(blknm,&fn[6]);
		/*
							 * Do stat on block device name in blknm
							 */
		if((rc = stat(blknm,&st) != 0)) {
			do {
				printf("Unknown character special device: %s\n",fn);
				printf("Device must not contain a Mounted File System\n");
				printf("Do you wish to continue?  (y/n): ");
				scanf("%s",ans); 
			} 
			while ((ans[0] != 'y')&&(ans[0] != 'n'));
			if (ans[0] == 'n') exit(1);
			sync();
			return(0);
		}
	}
	/*
				 * Here st.st_rdev should contain the raw device number of block special
				 * device, see if file system mounted on it.
				 */
	rc = ustat((int)st.st_rdev,&ust);
	if(rc == 0) {
		printf("WARNING ***\nDevice %s contains a mounted file system\n",fn);
		printf("Output device will be corrupted if any file activity occurs\n");
		do {
			printf("Do you wish to continue ? (y/n): ");
			scanf("%s",ans);
		} 
		while((ans[0] != 'y')&&(ans[0] != 'n'));
		if(ans[0] == 'n') exit(1);
		sync();
	}
	return(0);
}

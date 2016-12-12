#include "types.h"
#include "stat.h"
#include "user.h"



void printFstat(char* path) {
	
	struct stat st;
	int result = stat(path, &st);
	if (result < 0) {
		printf(1, "There is something wrong!");
	} else {
		printf(1, "Device number: %d\n", st.dev);
		printf(1, "Inode number on device: %d\n", st.ino);
		printf(1, "Number of links to file: %d \n", st.nlink);
		printf(1, "Size of file in bytes: %d \n", st.size);
		printf(1, "Checksum of file: %c\n", st.checksum);
	}
	
}
int main(int argc, char * argv[]) {
	
	if (argc < 2) {
		printf(1, "Run as filestat pathname\n");
	} else {
		printFstat(argv[1]);
	}
	exit();
}
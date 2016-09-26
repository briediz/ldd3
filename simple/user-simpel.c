#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>


#define FILESIZE 4096
int main(int argc, char* argv[]) {
 
    int input_fd;
    int *map, *ptr;
    int i, fd;
    
    /* Are src and dest file name arguments missing */
    if(argc != 2){
        printf ("Usage: file \n");
        return 1;
    }
 
    /* Create input file descriptor */
    input_fd = open (argv [1], O_RDONLY);
    if (input_fd == -1) {
        perror ("open");
        return 2;
    }
    
    map = mmap(0, FILESIZE, PROT_READ , MAP_SHARED, input_fd, 0);
    ptr=map;
    if (map <= 0) {
        close(input_fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    } else {
        printf ("mapping ok\n");
    }

    printf("val=0x%08d\n", map[0] );
    printf("val=0x%08d\n", map[1] );
   
    if (munmap(map, FILESIZE) == -1) {
        perror("Error un-mmapping the file");
    }
    
    close (input_fd);

#if 0
    fd = open("/sys/bus/pci/devices/0000:01:00.0/resource1", O_RDWR | O_SYNC);
    ptr = mmap(0, 40960000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    for( i=0; i<10; i++){
        printf("PCI BAR[%d] 0x0000 = 0x%x\n", i,  *(ptr));
        ptr++;
    }
    close (fd);
#endif 
    return (EXIT_SUCCESS);
}

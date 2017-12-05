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


#define FILESIZERAM   0x10000000
#define FILESIZECONF  0x00020000
#define FILESIZEFLASH 0x02000000 

int main(int argc, char* argv[]) {
 
    int input_fd;
    int *map, *ptr;
    int  fd, map_length, tmp;
    unsigned long long i, offset, length;
    
    if(argc != 4){
        printf ("Usage: read-programm device offset length\n");
        return 1;
    }
 
    if(strcmp(argv [1], "/dev/ram")==0)
        map_length=FILESIZERAM;
    else if(strcmp(argv [1], "/dev/conf")==0)
        map_length=FILESIZECONF;
    else if(strcmp(argv [1], "/dev/flash")==0)
        map_length=FILESIZEFLASH;
    else {
        perror("Error set file size");
        exit(EXIT_FAILURE);
        return -1;
    }

    input_fd = open (argv [1], O_RDONLY);
    if (input_fd == -1) {
        perror ("open");
        return 2;
    }
    
    map = mmap(0, map_length, PROT_READ , MAP_SHARED, input_fd, 0);
    ptr=map;
    if (map <= 0) {
        close(input_fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    } else {
        printf ("mapping ok, file open %s, size 0x%x\n", argv[1], map_length);
    }
    
    if (argv [2] != NULL)
        offset =  atoi(argv[2]);
    else
        offset = 0;
    
    
    if (argv [3] != NULL)
        length = atoi(argv[3]);
    else
        length = 10;    
    
     
    for( i=offset; i<offset+length; i++){
        printf("val[%08x]=0x%08x\n",i, map[i] );
    }
    
    printf("read done %x bytes\n", map_length );
    
    if (munmap(map, map_length) == -1) {
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/**
 * thirddrvtest.c 
 */
int main(int argc, char const *argv[])
{
    int fd;
    unsigned char key_val;

    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0)
    {
        perror("can't open");
        return -1;
    }

    while ( 1 )
    {
        read(fd, &key_val, 1); // read函数会一直阻塞等待，之后读取到数据才会返回，进而才会进行后面的print
        printf("key_val = 0x%x\n", key_val);
    }
    
    
    return 0;
}

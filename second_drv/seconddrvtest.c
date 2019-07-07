#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/**
 * seconddrvtest
 */

int main(int argc, char const *argv[])
{
    int fd;
    unsigned char key_val[4];
    int cnt = 0;

    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0)
    {
        printf("can't open!\n");
        return -1;
    }
    while (1)
    {
        read(fd, key_val, sizeof(key_val));
        if (!key_val[0] || !key_val[1] | !key_val[2] | !key_val[3])
        {   
            printf("%04d key pressed: %d %d %d %d\n",\
             cnt++, key_val[0], key_val[1],key_val[2],key_val[3]);
        }
        
    }
    return 0;
}

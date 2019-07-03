#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/**
 * firstdrvtest on
 * firstdrvtest off
 */

int main(int argc, char const *argv[])
{
    int fd;
    int val = 1;
    fd = open("/dev/xyz", O_RDWR);
    if (fd < 0 )
    {
        printf("can not open!\n");
        return -1;
    }
    if (argc != 2 )
    {
        printf("usage:\n");
        printf("%s <on|off>\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "on") == 0)
    {
        val = 1;
    }
    else 
    {
        val = 0;
    }

    write(fd, &val, 4);
    
    return 0;
}

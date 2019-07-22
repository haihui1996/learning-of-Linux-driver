#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>

/**
 * forthdrvtest
 * poll 机制解决了read函数一直不返回的情况，就是
 */

int main(int argc, char const *argv[])
{
    int fd;
    unsigned char key_val;
    int ret;
    
    struct pollfd fds[1];

    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0)
    {
        perror("can't open!\n");
        return -1;
    }

    fds[0].fd   = fd;
    fds[0].events = POLLIN; // 有数据可供读取时返回

    while (1)
    {
        /**
         * 当有数据可供读取时返回1，否则等待5000ms内没有数据则返回0
         * 这方法解决了read函数一直阻塞等待数据而不返还的情况，
         * 5秒钟之后没有数据的话，就可以不调用read函数，而去执行其他操作了
         */
        ret = poll(fds, 1, 5000);
        if (ret == 0)
        {
            printf("time out \n"); 
        }
        else
        {
            read(fd, &key_val, 1);
            printf("key_val = 0x%x\n",key_val);
        }   
    }
    
    return 0;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>

/**
 * forthdrvtest
 * poll ���ƽ����read����һֱ�����ص����������
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
    fds[0].events = POLLIN; // �����ݿɹ���ȡʱ����

    while (1)
    {
        /**
         * �������ݿɹ���ȡʱ����1������ȴ�5000ms��û�������򷵻�0
         * �ⷽ�������read����һֱ�����ȴ����ݶ��������������
         * 5����֮��û�����ݵĻ����Ϳ��Բ�����read��������ȥִ������������
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

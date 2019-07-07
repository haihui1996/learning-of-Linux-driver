KERN_DIR = /home/haihui/kernel/linux-2.6.32.2

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= hello.o

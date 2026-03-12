
obj-m += mailbox.o
mailbox-objs := mb_main.o mb_rw.o mb_proc.o 


all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean



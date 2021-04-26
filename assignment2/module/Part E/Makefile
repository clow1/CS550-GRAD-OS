# Simple Makefile to build a simple misc driver
# Nick Glynn <Nick.Glynn@feabhas.com>
#

obj-m += process_list.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
	$(CC) user_space_prog.c -o user_space_prog
        
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm user_space_prog
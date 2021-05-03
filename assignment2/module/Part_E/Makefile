obj-m := process_list.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(CC) user_space_prog.c -o user_space_prog

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	rm user_space_prog

.PHONY: modules clean

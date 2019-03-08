obj-m = realtk8139.o
KVERSION = linux-2.4.18

all:
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

obj-m := driver.o

KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
	gcc -o user_app user_app.c

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~
	rm -rf user_app

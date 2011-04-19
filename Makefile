
#obj-$(CONFIG_UPLO_CASE) += uplo_case.o

CONFIG_UPLO_DRIVER=m
obj-$(CONFIG_UPLO_DRIVER) += uplo_driver.o

all:
	CONFIG_UPLO_DRIVER=m
	CONFIG_UPLO_CASE=m
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


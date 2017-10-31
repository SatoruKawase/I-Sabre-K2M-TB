KERNEL_SRC = /lib/modules/$(shell uname -r)/source
BUILD_DIR := $(shell pwd)
DTC_DIR = /lib/modules/$(shell uname -r)/build/scripts/dtc/
VERBOSE = 0

OBJS    = i-sabre-codec.o i-sabre-k2m.o

obj-m := $(OBJS)

all:
	make -C $(KERNEL_SRC) SUBDIRS=$(BUILD_DIR) KBUILD_VERBOSE=$(VERBOSE) modules

clean:
	make -C $(KERNEL_SRC) SUBDIRS=$(BUILD_DIR) clean
	rm -f i-sabre-k2m.dtbo

dtbs:
	$(DTC_DIR)/dtc -@ -I dts -O dtb -o i-sabre-k2m.dtbo i-sabre-k2m-overlay.dts

modules_install:
	mkdir -p /lib/modules/$(shell uname -r)/kernel/sound/soc/codecs/
	cp i-sabre-codec.ko /lib/modules/$(shell uname -r)/kernel/sound/soc/codecs/
	mkdir -p /lib/modules/$(shell uname -r)/kernel/sound/soc/rockchip
	cp i-sabre-k2m.ko /lib/modules/$(shell uname -r)/kernel/sound/soc/rockchip/
	depmod -a

modules_remove:
	rm /lib/modules/$(shell uname -r)/kernel/sound/soc/codecs/i-sabre-codec.ko
	rm /lib/modules/$(shell uname -r)/kernel/sound/soc/rockchip/i-sabre-k2m.ko
	depmod -a

install:
	modprobe i-sabre-codec
	modprobe i-sabre-k2m

remove:
	modprobe -r i-sabre-k2m
	modprobe -r i-sabre-codec

install_dtbo:
	cp i-sabre-k2m.dtbo /boot/

remove_dtb:
	rm -f /boot/i-sabre-k2m.dtbo

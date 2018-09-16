
CROSS_COMPILE = arm-linux-
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)g++
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS := -Wall -O2 -g
CFLAGS += -I $(shell pwd)/../../include

LDFLAGS := -lm -lfreetype -lpthread -ljpeg -lpng -lasound -lmad -lid3tag -lopencv_core -lrt -lopencv_objdetect -ltxdevicesdk  -lpthread  -ldl  -lstdc++

export CFLAGS LDFLAGS

CURDIR := $(shell pwd)
TOPDIR := $(shell pwd)/../..
export TOPDIR
export CURDIR

TARGET := inputmanager

obj-y += main_inputmanager.o
obj-y +=  $(TOPDIR)/frameworks/
obj-y +=  $(TOPDIR)/system/


all :
	make -C ./ -f Makefile.build
	$(CC) $(LDFLAGS) -o $(TARGET) built-in.o
	mv $(TARGET) $(TOPDIR)/bin

clean:
	rm -f $(shell find $(TOPDIR) -name "*.o")
	rm -f $(TOPDIR)/bin/$(TARGET)

distclean:
	rm -f $(shell find $(TOPDIR) -name "*.o")
	rm -f $(shell find $(TOPDIR) -name "*.d")
	rm -f $(TOPDIR)/bin/*
	

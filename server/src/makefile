

LIBNAME  ?= server
#CC        = gcc 
LIBPATH  ?= ../libs/
ICLPATH  ?= ../include/

BUILD_ARCH=$(shell uname -m)
ifneq ($(findstring $(BUILD_ARCH), "x86_64" "i386"),)
	CROSS_COMPILE=arm-linux-gnueabihf-
endif

all: clean
	@gcc -shared -fPIC *.c -o lib${LIBNAME}.so
	@make install

arm-linux-gnueabihf-gcc: clean
	@arm-linux-gnueabihf-gcc -shared -fPIC *.c -o lib${LIBNAME}.so
	@make install

arm-linux-gnueabihf-gcc-static: clean
	@arm-linux-gnueabihf-gcc -c *.c
	@arm-linux-gnueabihf-ar -rcs lib${LIBNAME}.a *.o
	@make install
	@make clear

static: clean
	@${CROSS_COMPILE}gcc -c *.c
	@ar -rcs lib${LIBNAME}.a *.o
	@make install
	@make clear

clean:
	@make clear
	@rm -f ${LIBPATH}lib${LIBNAME}.*
	@rm -f ${ICLPATH}*

clear:
	@rm -f *.o

install:
	@mkdir -p ${LIBPATH}
	@mkdir -p ${ICLPATH}
	@mv lib${LIBNAME}.* ${LIBPATH}
	@cp *.h ${ICLPATH}

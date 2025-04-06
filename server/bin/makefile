

LIBNAME  ?= server

all: clean
	@gcc -shared -fPIC -I ../include/ *.c -o lib${LIBNAME}.so
	@make install
	@mv lib${LIBNAME}.so ../libs

arm-linux-gnueabihf-gcc: clean
	@arm-linux-gnueabihf-gcc -shared -fPIC -I ../include/ *.c -o lib${LIBNAME}.so
	@make install
	@mv lib${LIBNAME}.so ../libs

arm-linux-gnueabihf-gcc-static: clean
	@arm-linux-gnueabihf-gcc -I ../include -c *.c
	@arm-linux-gnueabihf-ar -rcs lib${LIBNAME}.a *.o
	@make install
	@mv lib${LIBNAME}.a ../libs
	@make clear

static: clean
	@gcc -I ../include -c *.c
	@ar -rcs lib${LIBNAME}.a *.o
	@make install
	@mv lib${LIBNAME}.a ../libs
	@make clear

clean:
	@make clear
	@rm -f ../libs/*

clear:
	@rm -f *.o

install:
	@mkdir -p ../libs

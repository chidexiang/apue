
LIBNAME  ?= client

all: clean
	gcc -shared -fPIC -I ../include/ *.c -o lib${LIBNAME}.so
	@make install
	@mv lib${LIBNAME}.so ../libs

static: clean
	gcc -c *.c
	@make clear
	ar -rcs -I ../include/ lib${LIBNAME}.a *.o
	@make install
	@mv lib${LIBNAME}.a ../libs

clean:
	@make clear
	@rm -f *.so
	@rm -f *.a

clear:
	@rm -f *.o

install:
	@mkdir -p ../libs

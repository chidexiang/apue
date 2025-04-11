
NAME     ?= socket_server
CC       ?= gcc 
ICLPATH   = ./include/
ICLFLAG   = -I ${ICLPATH}
LIBPATH   = ./libs/
LIBFLAG   = -L ${LIBPATH} -lserver -lsqlite3 -lpthread
BINPATH   = ./bin/

all:
	@make -C src
	@${CC} ${NAME}.c ${ICLFLAG} -o ${NAME} ${LIBFLAG}	            
	@make install

arm-linux-gnueabihf-gcc:
	@make arm-linux-gnueabihf-gcc -C src
	@arm-linux-gnueabihf-gcc ${NAME}.c ${ICLFLAG} -o ${NAME} ${LIBFLAG}
	@make install

arm-linux-gnueabihf-gcc-static:
	@make arm-linux-gnueabihf-gcc-static -C src
	@arm-linux-gnueabihf-gcc ${NAME}.c ${ICLFLAG} -o ${NAME} ${LIBFLAG}
	@make install

static:
	@make static -C src
	@${CC} ${NAME}.c ${ICLFLAG} -o ${NAME} ${LIBFLAG}
	@make install

install:
	@mkdir -p ${BINPATH}
	@mv ${NAME} ${BINPATH}

clean:
	@rm -f ${BINPATH}${NAME}

delect_include:
	@rm -rf ${ICLPATH}

delect_bin:
	@rm -rf ${BINPATH}

delect_lib:
	@rm -f ./libs/libserver.*

cleanall:
	@make clean
	@make delect_bin
	@make delect_lib
	@make delect_include

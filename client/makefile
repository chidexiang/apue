
NAME     ?= socket_client
CC       ?= gcc
ICLPATH   = ./include/
ICLFLAG   = -I ${ICLPATH}
LIBPATH   = ./libs/
LIBFLAG   = -L ${LIBPATH} -lclient -lm -lsqlite3 -lpthread
BINPATH   = ./bin/

#run:export LD_LIBRARY_PATH=`pwd`/../libs:$LD_LIBRARY_PATH

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
	@cp test_clients.sh ${BINPATH}

clean:
	@rm -f ${BINPATH}${NAME}

delect_include:
	@rm -rf ${ICLPATH}

delect_bin:
	@rm -rf ${BINPATH}

delect_lib:
	@rm -f ./libs/libclient.*

cleanall:
	@make clean
	@make delect_bin
	@make delect_lib
	@make delect_include

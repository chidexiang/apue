
NAME ?= socket_client

all:
	@make -C bin
	@gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lclient -lm -lsqlite3 -lpthread
#	@make run

arm-linux-gnueabihf-gcc:
	@make arm-linux-gnueabihf-gcc -C bin
	@arm-linux-gnueabihf-gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lclient -lsqlite3 -lm -lpthread

arm-linux-gnueabihf-gcc-static:
	@make arm-linux-gnueabihf-gcc-static -C bin
	@arm-linux-gnueabihf-gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lclient -lsqlite3 -lm -lpthread

static:
	@make static -C bin
	@gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lclient -lm -lsqlite3 -lpthread
#	@make run

run:
	@export LD_LIBRARY_PATH=$$(pwd)/libs:$$LD_LIBRARY_PATH && \
	./${NAME} -i 127.0.0.1 -p 11008 -t 2

clean:
	@rm -f ${NAME}

delect_txt:
	@rm -f client.txt

delect_db:
	@rm -f client.db

delect_lib:
	@rm -f ./libs/libclient.*

cleanall:
	@make clean
	@make delect_txt
	@make delect_lib
	@make delect_db

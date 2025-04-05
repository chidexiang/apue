


NAME ?= socket_server

all:
	@make -C bin
	@export LD_LIBRARY_PATH=${PWD}/libs:$$LD_LIBRARY_PATH
	@gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lserver -lsqlite3
	@make run

static:
	@make static -C bin
	@gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lserver -lsqlite3
	@make run

run:
	export LD_LIBRARY_PATH=$$(pwd)/libs:$$LD_LIBRARY_PATH && \
	./${NAME} -p 11008

clean:
	@rm -f ${NAME}

delect_txt:
	@rm -f server.txt

delect_lib:
	@rm -f ./libs/libserver.*
		         
cleanall:
	@make clean
	@make delect_txt
	@make delect_lib

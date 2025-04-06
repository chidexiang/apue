


NAME ?= socket_server

all:
	@make -C bin
	@gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lserver -ldl -lpthread
#	@make run

arm-linux-gnueabihf-gcc:
	@make arm-linux-gnueabihf-gcc -C bin
	@gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lserver -ldl -lpthread
#	@make run

arm-linux-gnueabihf-gcc-static:
	@make arm-linux-gnueabihf-gcc-static -C bin
	@gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lserver -ldl -lpthread
#	@make run

static:
	@make static -C bin
	@gcc ${NAME}.c -I ./include -o ${NAME} -L ./libs -lserver -ldl -lpthread
#	@make run

run:
	@export LD_LIBRARY_PATH=$$(pwd)/libs:$$LD_LIBRARY_PATH && \
	./${NAME} -p 11008

clean:
	@rm -f ${NAME}

delect_txt:
	@rm -f server.txt

delect_lib:
	@rm -f ./libs/*
		         
cleanall:
	@make clean
	@make delect_txt
	@make delect_lib

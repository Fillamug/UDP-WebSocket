.PHONY = all create_objects create_library link_library

CC = gcc

LINKS = -L. -ljsonsocket -ljson-c -pthread -Wl,-rpath,..

all: create_objects create_library link_library

create_objects:
	@echo "Creating objects"
	${CC} -o Server/UDPServer.o -c Server/UDPServer.c -fPIC
	${CC} -o Client/UDPClient.o -c Client/UDPClient.c -fPIC

create_library:
	@echo "Creating shared library"
	${CC} -shared -o libjsonsocket.so Server/UDPServer.o Client/UDPClient.o -lm

link_library:
	@echo "Linking library"
	${CC} Server/ChatServer.c ${LINKS} -o Server/server
	${CC} Client/ChatClient.c ${LINKS} -o Client/client

clear:
	@echo "Clearing up"
	find . \( -name "*.o" -o -name "*.so" -o -name "server" -o -name "client" \) -type f -delete

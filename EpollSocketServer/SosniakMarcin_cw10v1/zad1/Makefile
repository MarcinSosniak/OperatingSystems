all:
	gcc -Wall v1Client/main.c v1Client/data_structs_defs.c -o client -lpthread
	gcc -Wall v1Server/main.c v1Server/data_structs_defs.c v1Server/server.c v1Server/que.c  -o server -lpthread
clean:
	rm client; rm server
	

all:
	gcc -Wall v2Client/main.c v2Client/data_structs_defs.c -o client -lpthread
	gcc -Wall v2Server/main.c v2Server/data_structs_defs.c v2Server/server.c v2Server/que.c  -o server -lpthread
clean:
	rm client; rm server
	

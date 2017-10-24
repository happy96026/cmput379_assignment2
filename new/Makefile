a2chat: a2chat.o Client.o Server.o error_handling.o
	g++ a2chat.o error_handling.o Client.o Server.o -o a2chat

a2chat.o: a2chat.cpp
	g++ -std=c++11 -c a2chat.cpp -o a2chat.o

Client.o: Client.cpp
	g++ -std=c++11 -c Client.cpp -o Client.o

Server.o: Server.cpp
	g++ -std=c++11 -c Server.cpp -o Server.o

error_handling.o: error_handling.c
	gcc -c error_handling.c -o error_handling.o

tar:
	tar -czvf submit.tar ./Makefile ./README.txt ./a2chat.cpp ./Constants.h ./Client.cpp ./Client.h ./Server.cpp ./Server.h ./error_handling.c ./error_handling.h

clean:
	rm -rf *.o

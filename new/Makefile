a2chat: Client.o error_handling.o a2chat.cpp
	g++ -std=c++11 -c a2chat.cpp -o a2chat.o
	g++ a2chat.o error_handling.o Client.o -o a2chat

Client.o: Client.cpp
	g++ -std=c++11 -c Client.cpp -o Client.o

Server.o: Server.cpp
	g++ -std=c++11 -c Server.cpp -o Server.o

error_handling.o: error_handling.c
	gcc -c error_handling.c -o error_handling.o

clean:
	rm -rf *.o

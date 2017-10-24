Name: Min Soung(Daniel) Choi
csid: minsoung
ID: 1438979


Objective
	The objective of the program is to develop and create a program that allows for users to communicate with other
	clients through the terminal. The server is responsible for receiving the data from clients through FIFOs and 
	send it to the correct recipients.

Design Overview
	For client:
		-client can open new connection with the server by looking for an unlocked fifo
		-client can add recipients by "to"
		-client can send message by "< the_message_you_want_to_send"
		-client can close session by "close" and exit out of the program by "exit"
	For server:
		-server monitors if any connection requests are made by the client
		-server can accept requests or decline requests when the username is already in the server or
		the maximum number of clients has been reached.
		-server can exit the program by "exit"

Project Status
	The program can only be executed in these conditions:
		-the named pipes must be created before running the program
		-the named pipes must be created such that the name matches the fifo name argument
		-assuming that the fifo name is "fifo", the fifo must be created starting from "fifo-1.in", "fifo-1.out",
		 "fifo-2.in", "fifo-2.out" and onwards
		-the named pipes must be created in the same directory as a2chat exec
		-constant NUMFIFO in Constants.h file must match the number of pipes in pairs
	The software does not handle interrupt and quit signals as the program's behaviour undefined after any of the
	interrupt signal occurs.

Testing and Results
	The program was manually testing. To test the boundary cases of the number of named pipes available and number of clients
	available, the number of client was set to 1 to test if maximum client could have been reached and NUMFIFO was set to 1
	to test if there were no more named pipes available to the user. Testing if username is already in use has been tested by
	trying to open sessions with the same username. For more sophisticated test, two sessions were opened and one of the session
	added the other session to its recipient list. Then, that other session was closed and reopened with the same username. Then
	the original session sends message without adding to its recipient list to check if the username has been deleted when
	the user closes the session. The following tests that has been mentioned passes against this program.

Acknowledgements
	Google, StackOverflow, Advanced Programming in the Unix Environment, code of AWK programming language

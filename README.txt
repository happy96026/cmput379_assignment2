Name: Min Soung(Daniel) Choi
ID: 1438979

The program a2rchat is a restricted version of the a2chat program.

The program currently has the following features:

	-open username command without the feature of being able
	 detect that other client is using the specified username or
	 that the client already has a session going

	-After the client connects to the user, the server echos
	 back to the user the received chatlines

	-close and exit command

THe program does not signals as the program is likely to not
behave properly when program is terminated using ctrl + Z or ctrl + C.

Before the program is ran, the user must create pairs of named pipes using
mkfifo command. Then, the user must change the constant variable NUMFIFO
in a2rchat.cpp to the number of pairs of pipes available. The name of the
pipes are [fifo]-[n].in and [fifo]-[n].out where [fifo] can be replaced
with any name and [n] is a number starting from 1, increasing by 1 as the
pairs are created. The user should create a maximum of five pairs of fifos.

To run the server:

	./a2rchat -s fifo nclient

where nclient is the maximum number of clients that can be connected to
the server. Note that the maximum number of clients that can be inputted
is NMAX = 5.

To run the client:
	
	./a2rchat -c fifo

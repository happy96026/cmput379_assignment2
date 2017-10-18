#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
using namespace std; 

extern "C" {
	#include "error_handling.h"
}


// Struct to store file descriptor values
typedef struct {
	int in;
	int out;
}	fd_t;


// Constant Declaration
const int NMAX 		 = 5;			// Maximum Number of Clients
const int NUMFIFO    = 5;			// Number of pairs of FIFOs
const int MAX_LENGTH = 100;			// Maximum Length of String

// Global Variable Declaration
fd_t   		   server_fd[NUMFIFO + 1];	// File Descriptor for Server
vector<string> server_username; 		// Username that is connected to server
int    		   server_clients = 0;		// Number of clients connected to server
fd_t   		   client_fd;				// File Descriptor for Client
int    		   client_session = 0;		// Client Session Status


//-------------------------------Helper Functions-------------------------------------
/*
   Splits a string by space and returns the result in a vector.
   Copied from https://www.quora.com/How-do-I-split-a-string-by-space-into-an-array-in-c++
*/
vector<string> split_by_space(string s) {
	vector<string> result;
	istringstream iss(s);
	for (string s; iss >> s;) {
		result.push_back(s);
	}

	return result;
}


/*
   Checks if string str matches any string in vector<string> vec.
   Returns 1 if true and 0 if false.
*/
int checkStrInVector(vector<string> vec, string str) {
	for (int i = 0; i < vec.size(); i++) {
		if (vec[i] == str) {
			return 1;
		}
	}
	return 0;
}


/*
   Returns the number of "" string found in vector<string> vec.
*/
int countNonEmptyString(vector<string> vec) {
	int count = 0;

	for (int i = 0; i < NUMFIFO; i++) {
		if (vec[i] != "") {
			count++;
		}
	}
	return count;
}


/*
   Sets the file descriptor set fds with the file descriptors
   given by server_fd[n].in
*/
void init_fd_set(fd_set *fds) {
	FD_ZERO(fds);
	for (int i = 0; i < NUMFIFO + 1; i++) {
		if (server_fd[i].in > -1) {
			FD_SET(server_fd[i].in, fds);
		}
	}
}


/*
   Returns the value for the nfds argument for select function
*/
int get_nfds(fd_t *fd) {
	int nfds;

	for (int i = 0; i < NUMFIFO + 1; i++) {
		if ((fd + i)->in > nfds) {
			nfds = (fd + i)->in;
		}
	}

	return nfds + 1;
}
//-------------------------------Client Side-------------------------------------
/*
   The loop that contains the client side of the program.
   Returns 1 if loop is repeated, 0 if not.
*/
int clientLoop(string fifo) {
	string buffer_str;
	vector<string> buffer_v;
	int size;
	char buffer[MAX_LENGTH];
	int  buffer_size = 0;

	cout << "a2chat_client: ";
	getline(cin, buffer_str);

	buffer_v = split_by_space(buffer_str);

	if (buffer_v.size() == 1 && buffer_v[0] ==  "exit") {
			if (client_session == 1) {
				write(client_fd.in, buffer_str.c_str(), buffer_str.length() + 2);
				close(client_fd.in);
				close(client_fd.out);
			}
			return 0;
	} else if (buffer_v.size() == 1 && buffer_v[0] == "close") {
		if (client_session = 1) {
			cout << "Closing current session...\n";
			write(client_fd.in, buffer_str.c_str(), buffer_str.length() + 2);
			lockf(client_fd.in, F_ULOCK, MAX_LENGTH);
			close(client_fd.in);
			close(client_fd.out);
			client_session = 0;
		} else {
			cout << "Session already closed!\n";
		}
	} else if (buffer_v.size() == 2 && buffer_v[0] == "open") {
		string fifo_in, fifo_out;

		for (int i = 1; i < NUMFIFO + 1; i++) {
			fifo_in  = fifo + "-" + to_string(i) + ".in";
			fifo_out = fifo + "-" + to_string(i) + ".out";

			client_fd.in = open(fifo_in.c_str(), O_WRONLY | O_NONBLOCK);

			if (lockf(client_fd.in, F_TLOCK, MAX_LENGTH) == 0) {

				// Found an available FIFO
				client_fd.out = open(fifo_out.c_str(), O_RDONLY | O_NONBLOCK);
				write(client_fd.in, buffer_str.c_str(), buffer_str.length() + 1);
				
				memset(buffer, 0, MAX_LENGTH);
				while (buffer_size <= 0) {
					buffer_size = read(client_fd.out, buffer, MAX_LENGTH);
				}
				
				buffer_str = buffer;

				if (buffer_str == "Success") {
					client_session = 1;
					cout << "FIFO [fifo-" << i << ".in] has been successfully locked ";
					cout << "by PID [" << getpid() << "]\n";
				} else if (buffer_str == "Limit") {
					WARNING("Reached client limit!\n");
					close(client_fd.out);
					return 0;
				}

				return 1;
			}
		}
		cout << "No FIFO available at the moment" << "\n";
		return 1;
	} else if (client_session == 0) {
		return 1;
	} else {
		write(client_fd.in, buffer_str.c_str(), buffer_str.length() + 2);
		memset(buffer, 0, MAX_LENGTH);
		while (buffer_size <= 0) {
			buffer_size = read(client_fd.out, buffer, MAX_LENGTH);
		}
		buffer_str = buffer;
		cout << "[Server]: " << buffer_str << "\n";
	}

	return 1;
}


/*
   Handles all the client tasks
*/
void client(string fifo) {
	cout << "Chat client begins\n";
	while (clientLoop(fifo)) {}
}
//-------------------------------Server Side-------------------------------------
/*
   The loop that contains the server side of the program.
   Returns 1 if repeated, 0 if terminated.
*/
int serverLoop(int nclient, string fifo) {
	char   buffer[MAX_LENGTH];
	int    buffer_size, ready;
	fd_set readfds;

	init_fd_set(&readfds);

	// Monitor the available FIFO.ins
	int n = select(get_nfds(server_fd), &readfds, NULL, NULL, NULL);

	for (int i = 0; i < NUMFIFO + 1; i++) {
		if (FD_ISSET(server_fd[i].in, &readfds) > 0) {
			memset(buffer, 0, MAX_LENGTH);

			buffer_size = read(server_fd[i].in, buffer, MAX_LENGTH);

			// index of file descriptor array that is ready
			ready = i;	

			if (buffer_size > 0) {
				string buffer_str(buffer);
				vector<string> buffer_v;
				buffer_v = split_by_space(buffer);

				// Data is received from stdin
				if (ready == 0) {
					if (buffer_v.size() == 1 && buffer_v[0] == "exit") {
						return 0;
					} else {
						cout << "Invalid argument(s)\n";
					}
					cout << "a2chat_server: " << flush;
				}
				// Data is received from FIFO
				else { 
					if (buffer_v.size() == 2 && buffer_v[0] == "open") {
						string fifo_out;

						fifo_out = fifo + "-" + to_string(ready) + ".out";
						server_fd[ready].out = open(fifo_out.c_str(), O_WRONLY, O_NONBLOCK);

						if (countNonEmptyString(server_username) >= nclient) {
							write(server_fd[ready].out, "Limit", 7);
							close(server_fd[ready].out);
						} else {
							write(server_fd[ready].out, "Success", 9);
							server_username[ready - 1] = buffer_v[1];
						}
					} else {
						if (buffer_v[0] == "close" || buffer_v[0] == "exit") {
							server_username[ready - 1] = "";
							close(server_fd[ready].out);
						} else {
							write(server_fd[ready].out, buffer_str.c_str(), buffer_str.length() + 2);
						}
					}
				}
			}
		}
	}

	return 1;
}


/*
   Handles all server tasks
*/
void server(int nclient, string fifo) {
	// Initialize File Descriptor values to -1
	for (int i = 0; i < NUMFIFO + 1; i++) {
		server_fd[i].in  = -1;
		server_fd[i].out = -1;
	}

	// Initialize Vector that contains all the username.
	// Not have been fully implemented
	server_username.reserve(NUMFIFO);
	for (int i = 0; i< NUMFIFO; i++) {
		server_username.push_back("");
	}

	// Standard I/O File Descriptors
	server_fd[0].in  = 0;
	server_fd[0].out = 1;

	// Open File Descriptors
	for (int i = 1; i < NUMFIFO + 1; i++) {
		string fifo_in;

		fifo_in = fifo + "-" + to_string(i) + ".in";

		server_fd[i].in = open(fifo_in.c_str(), O_RDONLY | O_NONBLOCK);
		if (server_fd[i].in == -1) {
			FATAL("Unable to open file descriptor\n");
		}
	}
	
	cout << "Chat server begins [nclient = " << nclient << "]\n";
	cout << "a2chat_server: " << flush;

	// Server Loop
	while (serverLoop(nclient, fifo)) {}
	
	// Close File Descriptors
	for (int i = 1; i < NUMFIFO + 1; i++) {
		close(server_fd[i].in );
	}
}
//-------------------------------Main Function-------------------------------------
/*
   The main function of the program
*/
int main(int argc, char *argv[]) {
	if (argc < 2) {
		WARNING("Too little argument(s)\n");
	} else {
		string arg(argv[1]);

		if (arg == "-s") {
			if (argc > 4) {
				WARNING("Too many argument(s)\n");
			} else if (argc < 4) {
				WARNING("Too little argument(s)\n");
			} else {
				int nclient;

				try {
					nclient = stoi(argv[3]);
					if (nclient > NMAX) {
						WARNING("The maximum number of nclient is %d! nclient = %d\n", NMAX, nclient);
					} else {
						server(nclient, argv[2]);
					}
				} catch (const invalid_argument& e) {
					WARNING("Invalid argument(s)\n");
				}
			}
		} else if (arg == "-c") {
			if (argc > 3) {
				WARNING("Too many argument(s)\n");
			} else if (argc < 3) {
				WARNING("Too little argument(s)\n");
			} else {
				client(argv[2]);
			}
		} else {
			WARNING("Invalid argument(s)\n");
		}
	}
}

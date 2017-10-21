#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "Client.h"
#include "Constants.h"

using namespace std; 

extern "C" {
	#include "error_handling.h"
}

// Global Variable Declaration
FileDesInOut   server_fd[NUMFIFO + 1];	// File Descriptor for Server
vector<string> server_username; 		// Username that is connected to server
int    		   server_clients = 0;		// Number of clients connected to server
FileDesInOut   client_fd;				// File Descriptor for Client
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
int get_nfds(FileDesInOut *fd) {
	int nfds;

	for (int i = 0; i < NUMFIFO + 1; i++) {
		if ((fd + i)->in > nfds) {
			nfds = (fd + i)->in;
		}
	}

	return nfds + 1;
}

void clientMain(string fifo) {

	Client client(fifo);
	string buffer;
	vector<string> bufferVector;

	cout << "Chat client begins\n";
	while (true) {
		cout << "a2chat_client: ";
		getline(cin, buffer);

		bufferVector = split_by_space(buffer);

		if (bufferVector[0] == "exit" && bufferVector.size() == 1) {
			client.exitClient();
			break;
		} else if (bufferVector[0] == "close" && bufferVector.size() == 1) {
			client.closeClient();
		} else if (bufferVector[0] == "open" && bufferVector.size() == 2) {
			client.openClient(buffer);
		} else {
			client.echo(buffer);
		}
	}

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
							write(server_fd[ready].out, "limit", 7);
							close(server_fd[ready].out);
						} else {
							write(server_fd[ready].out, "success", 9);
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
				clientMain(argv[2]);
			}
		} else {
			WARNING("Invalid argument(s)\n");
		}
	}
}

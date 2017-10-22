#include <iostream>
#include <sstream>
#include "Client.h"
#include "Server.h"
#include "Constants.h"

extern "C" {
	#include "error_handling.h"
}

using namespace std; 


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


//-------------------------------Client Side-------------------------------------
void clientMain(string fifo) {

	Client client(fifo);
	string buffer;
	vector<string> bufferVector;
	int readyFifo;

	cout << "Chat client begins" << endl;
	while (true) {
		cout << "a2chat_client: " << flush;

		readyFifo = client.monitorFifos(buffer);
		bufferVector = split_by_space(buffer);

		if (readyFifo == 0) {
			if (bufferVector[0] == "exit" && bufferVector.size() == 1) {
				client.exitClient();
				break;
			} else if (bufferVector[0] == "close" && bufferVector.size() == 1) {
				client.closeClient();
			} else if (bufferVector[0] == "open" && bufferVector.size() == 2) {
				client.openClient(buffer);
			} else if (bufferVector[0] == "who" || bufferVector[0] == "to") {
				client.sendToServer(buffer);
			} else if (bufferVector[0] == "<") {
				client.writeToFifo(client.getFdIn(), buffer);
			}
		} else {
			cout << "\n" << buffer << endl;
		}
	}

}
//-------------------------------Server Side-------------------------------------
void serverMain(string fifo, int nclient) {
	Server server(fifo, nclient);
	string buffer;
	vector<string> bufferVector;
	int readyFifo;

	cout << "Chat server begins [nclient = " << nclient << "]\n";
	cout << "a2chat_server: " << flush;
	while (true) {

		readyFifo = server.monitorFifos(buffer);
		bufferVector = split_by_space(buffer);

		if (readyFifo == 0) {
			if (bufferVector[0] == "exit" && bufferVector.size() == 1) {
				break;
			} else {
				cout << "a2chat_server: " << flush;
			}
		} else {
			if (bufferVector[0] == "open" && bufferVector.size() == 2) {
				if (server.isMaxClient()) {
					server.declineConnection(readyFifo, "limit");
				} else if (server.isUserInServer(bufferVector[1]) >= 0) {
					server.declineConnection(readyFifo, "user");
				} else {
					server.acceptConnection(readyFifo, bufferVector[1]);
				}
			} else if ((bufferVector[0] == "exit" || bufferVector[0] == "close") &&
					(bufferVector.size() == 1)) {
				server.closeConnection(readyFifo);
			} else if (bufferVector[0] == "who" && bufferVector.size() == 1) {
				server.writeToFifo(server.getFifoOutFd(readyFifo), server.getUserStr());
			} else if (bufferVector[0] == "to") {
				server.addRecipients(readyFifo, bufferVector);
			} else if (bufferVector[0] == "<") {
				server.sendToRecipients(readyFifo, buffer);
			}
		}
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
						serverMain(argv[2], nclient);
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

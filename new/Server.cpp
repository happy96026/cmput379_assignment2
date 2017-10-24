#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "Server.h"

extern "C" {
	#include "error_handling.h"
}

using namespace std;


/**
  Constructor for the Server class.

  @param fifo basename for the fifo
  @param nclient maximum number of clients
*/
Server::Server(string fifo, int nclient) {
	fds[0].in  = 0;
	fds[0].out = 1;

	for (int i = 1; i < NUMFIFO + 1; i++) {
		string fifo_in;

		fifo_in = fifo + "-" + to_string(i) + ".in";

		fds[i].in = open(fifo_in.c_str(), O_RDONLY | O_NONBLOCK);
		if (fds[i].in == -1) {
			FATAL("Unable to open file descriptor\n");
		}

		usernames.push_back("");
	}

	this->nuser   = 0;
	this->fifo    = fifo;
	this->nclient = nclient;
}


/**
  Monitor values from FIFOs. This function blocks until data is received.

  @param buffer_str stores value read from fifo to the variable

  @return the index of fifo that was read
*/
int Server::monitorFifos(string &buffer_str) {
	fd_set fdset;
	char buffer[MAX_LENGTH];
	int buffer_size;
	
	while (true) {
		initFdInSet(&fdset);
		select(getMaxFdIn() + 1, &fdset, NULL, NULL, NULL);

		for (int i = 0; i < NUMFIFO + 1; i++) {
			if (FD_ISSET(fds[i].in, &fdset) > 0) {
				memset(buffer, 0, MAX_LENGTH);
				buffer_size = read(fds[i].in, buffer, MAX_LENGTH);
				if (buffer_size > 0) {
					buffer_str = buffer;
					if (i == 0) {
						buffer_str = buffer_str.substr(0, buffer_str.length() - 1);
					}
					return i;
				}
			}
		}
	}
}


/**
  Accepts connection requested by the client.

  @param fifoIndex the index of the fifo that made the request
  @param username the username of the client that made the request
*/
void Server::acceptConnection(int fifoIndex, string username) {
	int fd_out;
	string fifo_out = fifo + "-" + to_string(fifoIndex) + ".out";

	fd_out = open(fifo_out.c_str(), O_WRONLY, O_NONBLOCK);
	writeToFifo(fd_out, "success");
	usernames[fifoIndex - 1] = username;
	nuser++;
	
	fds[fifoIndex].out = fd_out;
}


/**
  Declines connection requested by the client.

  @param fifoIndex the index of the fifo that made the request
  @param message the type of rejection
*/
void Server::declineConnection(int fifoIndex, string message) {
	int fd_out;
	string fifo_out = fifo + "-" + to_string(fifoIndex) + ".out";

	fd_out = open(fifo_out.c_str(), O_WRONLY, O_NONBLOCK);
	writeToFifo(fd_out, message);

	close(fd_out);
}


/**
  Closes connection of the client.

  @param fifoIndex the index of the fifo that wants to close the connection
*/
void Server::closeConnection(int fifoIndex) {
	close(fds[fifoIndex].out);
	userMap[usernames[fifoIndex - 1]].clear();
	for (int i = 0; i < NUMFIFO; i++) {
		if (usernames[i] != "") {
			vector<string> recipients = userMap[usernames[i]];
			for (vector<string>::iterator it = recipients.begin(); it != recipients.end();
					) {
				if (usernames[fifoIndex - 1] == *it) {
					recipients.erase(it);
				} else {
					it++;
				}
			}
			userMap[usernames[i]] = recipients;
		}
	}
	usernames[fifoIndex - 1] = "";
	nuser--;
}

/**
  Exits the program for server.
*/
void Server::exit() {
	for (int i = 1; i < NUMFIFO + 1; i++) {
		close(fds[i].in);
		close(fds[i].out);
	}
}


/**
  Checks if maximum number of client is reached.

  @return true if maximum number of client is reached, otherwise false
*/
bool Server::isMaxClient() {
	if (nuser >= nclient) {
		return true;
	}

	return false;
}


/**
  Getter for fd for fifo-x.out.

  @param index index is the x value

  @return file descriptor for fifo-x.out
*/
int Server::getFifoOutFd(int index) {
	return (fds[index].out);
}


/**
  Writes to fifo.
*/
void Server::writeToFifo(int fd, string buffer) {
	write(fd, buffer.c_str(), buffer.length() + 2);
}


/**
  Checks if user is connected with the server.

  @param username the username that is to be checked
  
  @return the index of fifo that the username is linked with
*/
int Server::isUserInServer(string username) {
	for (int i = 0; i < NUMFIFO; i++) {
		if (usernames[i] == username) {
			return i;
		}
	}

	return -1;
}


/**
  Gets the string for that lists all the user

  @return string that has the list of users
*/
string Server::getUserStr() {
	string userStr = "[server]: Current users: ";
	int user = 1;
	for (int i = 0; i < NUMFIFO; i++) {
		if (usernames[i] != "") {
			userStr = userStr + "[" + to_string(user) + "] " + usernames[i] + ", ";
			user++;
		}
	}

	return userStr.substr(0, userStr.length() - 2);
}


/**
  Add Recipients to the user list

  @param fifoIndex the index of fifo that is related to the user
  @param bufferVector vector that contains the username starting from index 1
*/
void Server::addRecipients(int fifoIndex, vector<string> bufferVector) {
	vector<string> recipients = userMap[usernames[fifoIndex - 1]];
	string addedRecipients = "[server]: Recipients added: ";

	for (int i = 1; i < bufferVector.size(); i++) {
		for (int j = 0; j < NUMFIFO; j++) {
			if (bufferVector[i] == usernames[j] &&
					!(stringInVector(recipients, bufferVector[i]))) {
				recipients.push_back(bufferVector[i]);
				addedRecipients = addedRecipients + bufferVector[i] + ", ";
			}
		}
	}

	writeToFifo(fds[fifoIndex].out, addedRecipients.substr(0, addedRecipients.length() - 2));
	userMap[usernames[fifoIndex - 1]] = recipients;
}


/**
  Checks if string is in vector

  @return true if string is in vector, otherwise false
*/
bool Server::stringInVector(vector<string> vector, string str) {
	for (int i = 0; i < vector.size(); i++) {
		if (vector[i] == str) {
			return true;
		}
	}
	return false;
}


/**
  Send message to all the recipients linked to the user

  @param fifoIndex the index that is linked to the user
  @param buffer the mssage to seend
*/
void Server::sendToRecipients(int fifoIndex, string buffer) {
	vector<string> recipients = userMap[usernames[fifoIndex - 1]];
	string message = "[" + usernames[fifoIndex - 1] + "]: ";
	int userIndex;
	for (int i = 0; i < recipients.size(); i++) {
		userIndex = isUserInServer(recipients[i]);
		if (userIndex >= 0) {
			writeToFifo(fds[userIndex + 1].out, message + buffer.substr(2, buffer.length() - 2));
		}
	}
}

/**
  Initializes FdSet for monitor.
*/
void Server::initFdInSet(fd_set *fdset) {
	FD_ZERO(fdset);
	for (int i = 0; i < NUMFIFO + 1; i++) {
		FD_SET(fds[i].in, fdset);
	}
}


/**
  Get the max file descriptor value within fds.

  @return max file descriptor value
*/
int Server::getMaxFdIn() {
	int maxfd = -1;

	for (int i = 0; i < NUMFIFO + 1; i++) {
		if (fds[i].in > maxfd) {
			maxfd = fds[i].in;
		}
	}

	return maxfd;
}

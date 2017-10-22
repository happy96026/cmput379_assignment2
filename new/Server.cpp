#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "Server.h"

extern "C" {
	#include "error_handling.h"
}

using namespace std;


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


void Server::acceptConnection(int fifoIndex, string username) {
	int fd_out;
	string fifo_out = fifo + "-" + to_string(fifoIndex) + ".out";

	fd_out = open(fifo_out.c_str(), O_WRONLY, O_NONBLOCK);
	writeToFifo(fd_out, "success");
	usernames[fifoIndex - 1] = username;
	nuser++;
	
	fds[fifoIndex].out = fd_out;
}


void Server::declineConnection(int fifoIndex, string message) {
	int fd_out;
	string fifo_out = fifo + "-" + to_string(fifoIndex) + ".out";

	fd_out = open(fifo_out.c_str(), O_WRONLY, O_NONBLOCK);
	writeToFifo(fd_out, message);

	close(fd_out);
}


void Server::closeConnection(int fifoIndex) {
	close(fds[fifoIndex].out);
	usernames[fifoIndex - 1] = "";
	nuser--;
}

void Server::exit() {
	for (int i = 1; i < NUMFIFO + 1; i++) {
		close(fds[i].in);
		close(fds[i].out);
	}
}


bool Server::isMaxClient() {
	if (nuser >= nclient) {
		return true;
	}

	return false;
}


int Server::getFifoOutFd(int index) {
	return (fds[index].out);
}


void Server::writeToFifo(int fd, string buffer) {
	write(fd, buffer.c_str(), buffer.length() + 2);
}


int Server::isUserInServer(string username) {
	for (int i = 0; i < NUMFIFO; i++) {
		if (usernames[i] == username) {
			return i;
		}
	}

	return -1;
}


string Server::getUserStr() {
	string userStr = "Current users: ";
	int user = 1;
	for (int i = 0; i < NUMFIFO; i++) {
		if (usernames[i] != "") {
			userStr = userStr + "[" + to_string(user) + "] " + usernames[i] + ", ";
			user++;
		}
	}

	return userStr.substr(0, userStr.length() - 2);
}

void Server::addRecipients(int fifoIndex, vector<string> bufferVector) {
	vector<string> recipients = userMap[usernames[fifoIndex - 1]];
	string addedRecipients = "Recipients added: ";

	for (int i = 1; i < bufferVector.size(); i++) {
		for (int j = 0; j < NUMFIFO; j++) {
			if (bufferVector[i] == usernames[j] && !(stringInVector(recipients, bufferVector[i]))) {
				recipients.push_back(bufferVector[i]);
				addedRecipients = addedRecipients + bufferVector[i] + ", ";
			}
		}
	}

	writeToFifo(fds[fifoIndex].out, addedRecipients.substr(0, addedRecipients.length() - 2));
	userMap[usernames[fifoIndex - 1]] = recipients;
}


bool Server::stringInVector(vector<string> vector, string str) {
	for (int i = 0; i < vector.size(); i++) {
		if (vector[i] == str) {
			return true;
		}
	}
	return false;
}


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

void Server::initFdInSet(fd_set *fdset) {
	FD_ZERO(fdset);
	for (int i = 0; i < NUMFIFO + 1; i++) {
		FD_SET(fds[i].in, fdset);
	}
}


int Server::getMaxFdIn() {
	int maxfd = -1;

	for (int i = 0; i < NUMFIFO + 1; i++) {
		if (fds[i].in > maxfd) {
			maxfd = fds[i].in;
		}
	}

	return maxfd;
}

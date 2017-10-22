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
					return i;
				}
			}
		}
	}
}



void Server::acceptConnection(int fifoIndex) {
	int fd_out;
	string fifo_out = fifo + "-" + to_string(fifoIndex) + ".out";

	fd_out = open(fifo_out.c_str(), O_WRONLY, O_NONBLOCK);
	writeToFifo(fd_out, "success");
	nuser++;
	
	fds[fifoIndex].out = fd_out;
}


void Server::declineConnection(int fifoIndex) {
	int fd_out;
	string fifo_out = fifo + "-" + to_string(fifoIndex) + ".out";

	fd_out = open(fifo_out.c_str(), O_WRONLY, O_NONBLOCK);
	writeToFifo(fd_out, "limit");

	close(fd_out);
}


void Server::closeConnection(int fifoIndex) {
	usernames[fifoIndex - 1] = "";
	nuser--;
	close(fds[fifoIndex].out);
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

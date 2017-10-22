#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "Client.h"

extern "C" {
	#include "error_handling.h"
}

using namespace std;


Client::Client(string fifo) {
	fd.in  = -1;
	fd.out = -1;
	session = OFF;
	this->fifo = fifo;
}


void Client::exitClient() {
	cout << "Exiting client..." << endl;
	if (session == ON) {
		write(fd.in, "exit", 6);
		lockf(fd.in, F_ULOCK, MAX_LENGTH);
		close(fd.in);
		close(fd.out);
	}
}


void Client::closeClient() {
	if (session == ON) {
		cout << "Closing current session..." << endl;
		session = OFF;
		write(fd.in, "close", 7);
		lockf(fd.in, F_ULOCK, MAX_LENGTH);
		close(fd.in);
		close(fd.out);
	} else {
		cout << "Session already closed!" << endl;
	}
}


void Client::openClient(string buffer) {
	int fifo_index;
	FileDesInOut fd;

	if (session == OFF) {
		fifo_index = findAvailableFifoIn(); 
		if (fifo_index < 0) {
			WARNING("No available FIFOs found at the moment!\n");
		} else {
			string fifo_in, fifo_out;
			char rdata[MAX_LENGTH];

			fifo_in  = fifo + "-" + to_string(fifo_index) + ".in";
			fifo_out = fifo + "-" + to_string(fifo_index) + ".out";

			fd.in = open(fifo_in.c_str(), O_WRONLY);
			fd.out = open(fifo_out.c_str(), O_RDONLY | O_NONBLOCK);

			writeToFifo(fd.in, buffer);
			buffer = readFromFifo(fd.out);
			
			if (buffer == "success") {
				session = ON;
				lockf(fd.in, F_LOCK, MAX_LENGTH);
				this->fd.in = fd.in;
				this->fd.out = fd.out;
				cout << "FIFO [fifo-" << fifo_index << ".in] has been successfully ";
				cout << "locked by PID [" << getpid() << "]\n";
			} else if (buffer == "limit") {
				WARNING("Reached client limit!\n");
				close(fd.in);
				close(fd.out);
			} else {
				WARNING("Connection could not have been made\n");
			}
		}
	} else {
		cout << "Session is already on!" << endl;
	}
}


void Client::echo(string buffer) {
	if (session == ON) {
		writeToFifo(fd.in, buffer);
		buffer = readFromFifo(fd.out);

		cout << "[Server]: " << buffer << endl;
	} else {
		WARNING("Not connected to server!\n");
	}
}


string Client::readFromFifo(int fd) {
	char buffer[MAX_LENGTH];
	string buffer_str;
	int buffer_size = 0;

	memset(buffer, 0, MAX_LENGTH);
	while (buffer_size <= 0) {
		buffer_size = read(fd, buffer, MAX_LENGTH);
	}

	buffer_str = buffer;

	return buffer_str;
}


void Client::writeToFifo(int fd, string buffer) {
	write(fd, buffer.c_str(), buffer.length() + 2);
}


int Client::findAvailableFifoIn() {
	string fifo_in;
	int fd_in;

	for (int i = 1; i < NUMFIFO + 1; i++) {
		fifo_in = fifo + "-" + to_string(i) + ".in";

		fd_in = open(fifo_in.c_str(), O_WRONLY);

		if (lockf(fd_in, F_TEST, MAX_LENGTH) == 0) {
			close(fd_in);
			return i;
		}
	}

	close(fd_in);
	return -1;
}

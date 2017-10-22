#ifndef CLIENT_H
#define CLIENT_H

#include "Constants.h"

class Client {

	private:
		std::string	 fifo;
		FileDesInOut fd;
		Session      session;

	public:
		Client(std::string fifo);
		void exitClient();
		void closeClient();
		void openClient(std::string buffer);
		void echo(std::string buffer);

	private:
		std::string readFromFifo(int fd);
		void   		writeToFifo(int fd, std::string buffer);
		int    		findAvailableFifoIn();

};

#endif

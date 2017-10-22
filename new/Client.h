#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include "Constants.h"

class Client {

	private:
		std::string	 fifo;
		FileDesInOut fd;
		Session      session;

	public:
		Client(std::string fifo);
		int  monitorFifos(std::string &buffer_str);
		void exitClient();
		void closeClient();
		void openClient(std::string buffer);
		void sendToServer(std::string buffer);
		void writeToFifo(int fd, std::string buffer);
		int  getFdIn();

	private:
		std::string readFromFifo(int fd);
		int    		findAvailableFifoIn();
		void		initFdInSet(fd_set *fdset);
		int			getMaxInVector(std::vector<int> vector);

};

#endif

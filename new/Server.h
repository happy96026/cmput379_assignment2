#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <map>
#include <string>
#include "Constants.h"

class Server {

	private:
		FileDesInOut 			 		   				fds[NUMFIFO + 1];
		std::vector<std::string> 		   				usernames;
		int						 		   				nuser;
		std::map<std::string, std::vector<std::string>> userMap;
		std::string			     		   				fifo;
		int						 		   				nclient;

	public:
		Server(std::string fifo, int nclient);
		int  		monitorFifos(std::string &buffer_str);
		void 		acceptConnection(int fifoIndex, std::string username);
		void 		declineConnection(int fifoIndex, std::string message);
		void 		closeConnection(int fifoIndex);
		void 		exit();
		bool 		isMaxClient();
		int  		getFifoOutFd(int index);
		void 		writeToFifo(int fd, std::string buffer);
		int 		isUserInServer(std::string username);
		std::string getUserStr();
		void		addRecipients(int fifoIndex, std::vector<std::string> bufferVector);
		bool		stringInVector(std::vector<std::string> vector, std::string str);
		void		sendToRecipients(int fifoIndex, std::string buffer);

	private:
		void initFdInSet(fd_set *fdset);
		int  getMaxFdIn();
		

};

#endif

#ifndef CONSTANTS_H
#define CONSTANTS_H

// Struct to store file descriptor values
typedef struct {
	int in;
	int out;
}	FileDesInOut;

typedef enum {
	OFF,
	ON
}	Session;

// Constant Declaration
const int NMAX 		 = 5;
const int NUMFIFO 	 = 5;
const int MAX_LENGTH = 100;

#endif

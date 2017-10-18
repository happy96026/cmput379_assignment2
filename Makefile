a2rchat: error_handling.o a2rchat.cpp
	g++ -std=c++11 -c a2rchat.cpp -o a2rchat.o
	g++ -o a2rchat a2rchat.o error_handling.o

error_handling.o: error_handling.c
	gcc -c error_handling.c -o error_handling.o

clean:
	rm -rf *.o

tar:
	tar -czvf submit.tar ./Makefile ./README.txt ./a2rchat.cpp ./error_handling.c ./error_handling.h ./a2answers.txt

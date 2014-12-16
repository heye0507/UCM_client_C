UCM: main.o client.o
	gcc -o UCM -g main.o client.o

main.o: main.c 
	gcc -o main.o -g -c -Wall -lm main.c

client.o: client.c client.h error.h
	gcc -o client.o -g -c -Wall client.c

clean:
	rm -f *.o *.txt UCM

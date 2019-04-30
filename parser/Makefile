parserlib: llist.o cparser.o src/HelperFunction.c src/JSHelperFunctions.c
	gcc -c -fpic -Iinclude src/JSHelperFunctions.c -o bin/JSHelperFunctions.o
	gcc -c -fpic -Iinclude src/HelperFunction.c -o bin/HelperFunction.o
	gcc -shared -o ../libcal.so bin/llist.o bin/cparser.o bin/HelperFunction.o bin/JSHelperFunctions.o

all: parser list

parser: cparser.o
	gcc -shared bin/CalendarParser.o -o bin/LinkedListAPI.o

cparser.o: src/CalendarParser.c
	gcc -c -fpic -Iinclude src/CalendarParser.c -o bin/cparser.o

llist.o: src/LinkedListAPI.c
	gcc -c -fpic -Iinclude src/LinkedListAPI.c -o bin/llist.o

cm: src/*.c
	gcc -std=c11 -Wall -g -Iinclude src/*.c -o bin/runMe

run: bin/runMe
	./bin/runMe

memtest : bin/runMe
	valgrind --leak-check=full -v ./bin/runMe

clean:
	rm bin/*.o bin/*.so

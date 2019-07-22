exe: main.o secondaryHashTable.o hashTable.o hashFunctions.o readingFromCSV.o
	gcc -o exe main.o secondaryHashTable.o hashTable.o hashFunctions.o readingFromCSV.o -static BF_64.a

readingFromCSV.o: readingFromCSV.c
	gcc -c readingFromCSV.c

hashFunctions.o : hashFunctions.c hashFunctions.h secondaryHashTable.h hashTable.h
	gcc -c hashFunctions.c

hashTable.o : hashTable.c hashTable.h records.h blocks.h hashFunctions.h
	gcc -c hashTable.c

secondaryHashTable.o : secondaryHashTable.c secondaryHashTable.h records.h blocks.h hashFunctions.h
	gcc -c secondaryHashTable.c

main.o : main.c 
	gcc -c main.c 

clean:
	rm -f exe main.o secondaryHashTable.o hashTable.o hashFunctions.o readingFromCSV.o
	rm *.txt
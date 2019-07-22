#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "records.h"
#include <sys/types.h>


void readingFromCSV(char * file, Record *records){

    FILE *stream = fopen(file, "r");
    if (stream == NULL){
        printf("ERROR stream is NULL\n");
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    Record record;
    int counter = 0;
    char *temp;

    while ((read = getline(&line, &len, stream)) != -1)
    {
        line[read - 2] = 0;
        temp = line;
        temp++;
        char *pch;

        pch = strtok(temp, ",");
        record.id = atoi(pch);

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.name, pch, sizeof(record.name));

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.surname, pch, sizeof(record.surname));

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.address, pch, sizeof(record.address));

        records[counter] = record;
        counter++;

        record.id = 0;
        memset(record.name, '\0', sizeof(record.name));
        memset(record.surname, '\0', sizeof(record.surname));
        memset(record.address, '\0', sizeof(record.address));

        /*
        printf("record.id == %d\n", record.id);
        printf("record.name == %s\n", record.name);
        printf("record.surname == %s\n", record.surname);
        printf("record.address == %s\n\n", record.address);
*/
    }

    free(line);
}
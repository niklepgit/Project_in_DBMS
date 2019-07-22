#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BF.h"
#include "hashTable.h"
#include "secondaryHashTable.h"
#include "hashFunctions.h"
#include "records.h"

void readingFromCSV(char * file, Record *records);

int main(void){

    Record *records;

    int number_of_buckets;
    printf("Give the number of buckets: ");
    scanf("%d",&number_of_buckets);

    int size_of_dataset_file;
    printf("Give the size of dataset's file: ");
    scanf("%d", &size_of_dataset_file);

    while((size_of_dataset_file != 1000) && (size_of_dataset_file != 5000) && (size_of_dataset_file != 10000) && (size_of_dataset_file != 15000)){
        printf("The size should be 1000 or 5000 or 10000 or 15000, please try again: ");
        scanf("%d", &size_of_dataset_file);
    }

    records = malloc (size_of_dataset_file*sizeof(Record));
    if (records == NULL){
        printf("Something terrible happended with malloc!\n");
        return -1;
    }

    switch (size_of_dataset_file){

        case 1000:  readingFromCSV("datasets/records1K.txt",records);
                    break;

        case 5000:  readingFromCSV("datasets/records5K.txt",records);
                    break;

        case 10000: readingFromCSV("datasets/records10K.txt",records);
                    break;

        case 15000: readingFromCSV("datasets/records15K.txt",records);
                    break;

    }
    BF_Init();
    HT_CreateIndex("PrimaryHashFile.txt", 'i', "id", 1, number_of_buckets);
    HT_info *ptr = HT_OpenIndex("PrimaryHashFile.txt");
    //printf("sizeof record == %d\n",sizeof(Record));

    SHT_CreateSecondaryIndex("SecondaryHashFile.txt", "name", 9, number_of_buckets, "PrimaryHashFile.txt");
    SHT_info *sht_ptr = SHT_OpenSecondaryIndex("SecondaryHashFile.txt");
    if (sht_ptr == NULL)
        printf("Error \n");

    
    for (int i = 0; i < size_of_dataset_file; i++)
    {
        int test = HT_InsertEntry(*ptr, records[i]);

        SecondaryRecord SecRecord;
        SecRecord.record.id = records[i].id;
        strncpy(SecRecord.record.name, records[i].name, 15);
        strncpy(SecRecord.record.surname, records[i].surname, 20);
        strncpy(SecRecord.record.address, records[i].address, 40);
        SecRecord.blockId = test;
        //printf("%s\n", SecRecord.record.name);
        int sht_insert = SHT_SecondaryInsertEntry(*sht_ptr, SecRecord);

        //printf("test == %d and i ==  %d and  sht_insert == %d\n\n", test, i, sht_insert);
    }
    
    printf("SHT_SecondaryGetAllEntries:\n");
    for (int i = 10 ; i < 20; i++)
    {    
        char name[50];
        sprintf(name,"name_%d",i);
        int l = SHT_SecondaryGetAllEntries(*sht_ptr, *ptr, name);
    }
    printf("HT_DeleteEntry:\n");
    for (int i = 13 ; i < 17 ; i++)
        HT_DeleteEntry(*ptr,&i);
    
    printf("SHT_SecondaryGetAllEntries:\n");
    for (int i = 10 ; i < 20; i++)
    {    
        char name[50];
        sprintf(name,"name_%d",i);
        int l = SHT_SecondaryGetAllEntries(*sht_ptr, *ptr, name);
    }


    printf("HT_GetAllEntries:\n");
    int p = HT_GetAllEntries(*ptr, &(records[5].id));
    printf("Blocks Read: %d\n", p);

    HashStatistics("PrimaryHashFile.txt");
    HashStatistics("SecondaryHashFile.txt");
    SHT_CloseSecondaryIndex(sht_ptr);
    HT_CloseIndex(ptr);
    
    free(records);

    return 0;
}


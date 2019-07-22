#ifndef __SHT__
#define __SHT__

#include "records.h"
#include "hashTable.h"

typedef struct SHT_info
{

    char attrName[40];
    int fileDesc;
    int attrLength;
    long int numBuckets;
    char fileName[20]; /*name of the file that has the first key*/

} SHT_info;

int SHT_CreateSecondaryIndex(char* sfileName, char* attrName, int attrLength, int numBuckets , char* fileName);
SHT_info* SHT_OpenSecondaryIndex(char* sfileName);
int SHT_CloseSecondaryIndex(SHT_info* header_info);
int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record);
int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void* value);

#endif //end of __SHT__
#ifndef __HT__
#define __HT__

#include "records.h"

typedef struct HT_info
{

    char attrName[40]; /*name of the field that has the key*/
    int fileDesc;
    char attrType;     /*type of the key , et 'c' , 'i' */
    int attrLength;    /*size of the type of the key*/
    long int numBuckets;

} HT_info;

int HT_CreateIndex(char * fileName, char attrType, char* attrName, int attrLength, int numBuckets);
HT_info* HT_OpenIndex(char* fileName);
int HT_CloseIndex(HT_info* header_info);
int HT_InsertEntry(HT_info header_info, Record record);
int HT_DeleteEntry(HT_info header_info, void* value);
int HT_GetAllEntries(HT_info header_info, void* value);


#endif //end of __HT__
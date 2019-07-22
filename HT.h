

#include "records.h"
#include "hashFunctions.h"

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
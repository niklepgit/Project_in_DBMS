#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hashFunctions.h"
#include "hashTable.h"
#include "BF.h"
#include "blocks.h"



int HT_CreateIndex(char *fileName, char attrType, char *attrName, int attrLength, int numBuckets){

    int     result_value;
    int     error = 0;
    int     file_desc;
    void    *block_array;
    Block   block;
    HT_info hash_table_info;
    
    // BF_CreateFile to create the file that we want
    result_value = BF_CreateFile(fileName);
    if (result_value < 0){
        BF_PrintError("Error in HT_CreateIndex at BF_CreateFile");
        error = -1;
    }

    // BF_OpenFile to open the file
    file_desc = BF_OpenFile(fileName);
    if (file_desc < 0){
        BF_PrintError("Error in HT_CreateIndex at BF_OpenFile");
        error = -1;
    }
    else{
    //printf("file_desc in BF_OpenFile at HT_CreateIndex is %d\n",file_desc);    
    }
    // BF_AllocateBlock to allocate the block we want for the HashTable and the Buckets
    
    result_value = BF_AllocateBlock(file_desc);
    if (result_value < 0){
        BF_PrintError("Error in HT_CreateIndex at BF_AllocateBlock");
        error = -1;
    }

    result_value = BF_ReadBlock(file_desc, 0, &block_array);
    if (result_value < 0){
        BF_PrintError("Error in HT_CreateIndex at BF_ReadBlock");
        error = -1;
    }

    hash_table_info.fileDesc    = file_desc;
    hash_table_info.attrType    = attrType;
    hash_table_info.attrLength  = attrLength;
    hash_table_info.numBuckets  = numBuckets;
    strncpy(hash_table_info.attrName,attrName,20);
    //we need memcpy to create a byteArray in order to write the block back to file on disk
    memcpy(block_array, &hash_table_info, sizeof(hash_table_info));

    result_value = BF_WriteBlock(file_desc, 0);
    if (result_value < 0){
        BF_PrintError("Error in HT_CreateIndex at BF_WriteBlock");
        error = -1;
    }

    block.nextBlock         = -1;
    block.numberOfRecords   =  0;
    for (int i = 1; i < numBuckets + 1; i++){
        result_value = BF_AllocateBlock(file_desc);
        if (result_value < 0){
            BF_PrintError("Error in HT_CreateIndex at BF_AllocateBlock");
            error = -1;
        }
        // BF_ReadBlock to get the block for HashTable from disk to ram
        result_value = BF_ReadBlock(file_desc, i, &block_array);
        if (result_value < 0){
            BF_PrintError("Error in HT_CreateIndex at BF_ReadBlock");
            error = -1;
        }

        block.blockId = i;

        memcpy(block_array,&block,sizeof(block));
        result_value = BF_WriteBlock(file_desc, i);
        if (result_value < 0){
            BF_PrintError("Error in HT_CreateIndex at BF_WriteBlock");
            error = -1;
        }
    }

    result_value = BF_CloseFile(file_desc);
    if (result_value < 0){
        BF_PrintError("Error in HT_CreateIndex at BF_CloseFile");
        error = -1;
    }

    return error;
}

HT_info *HT_OpenIndex(char *fileName){

    int result_value;
    int error = 0;
    int file_desc;
    void *block;
    HT_info *hash_table_info;
    HT_info *temp_ptr;

    // BF_OpenFile to open the file
    file_desc = BF_OpenFile(fileName);
    if (file_desc < 0)
    {
        BF_PrintError("Error in HT_OpenIndex at BF_OpenFile");
        error = -1;
    }

    // BF_ReadBlock to get the block for HashTable from disk to ram
    result_value = BF_ReadBlock(file_desc, 0, &block);
    if (result_value < 0)
    {
        BF_PrintError("Error in HT_OpenIndex at BF_ReadBlock");
        error = -1;
    }
    char  attrNamePtr[40];
    strncpy(attrNamePtr,(char *)block,40);

    if (strcmp(attrNamePtr,"id") == 0){
        hash_table_info = malloc(sizeof(HT_info));
        temp_ptr = (HT_info*)block;
        *hash_table_info = *temp_ptr;
    }
    else{
        error = -1;
    }


    if (error == -1){
        return NULL;
    }
    else{
        return hash_table_info;
    }
}

int HT_CloseIndex(HT_info *header_info){

    int result_value;
    int error = 0;

    result_value = BF_CloseFile(header_info->fileDesc);
    if (result_value < 0)
    {
        BF_PrintError("Error in HT_CloseIndex at BF_CloseFile");
        error = -1;
    }
    if (error != -1){
        free(header_info);
        header_info = NULL;
    }
    
    return error;
}

int HT_InsertEntry(HT_info header_info, Record record){
    int number_of_block;
    void *block;
    Block *block_ptr;
    int result_value;
    int error;
    int blockCounter;
    Block new_block;
    int flag  = 0;

    if (header_info.attrType == 'i'){
        number_of_block = intHashing(record.id, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "name")){
        number_of_block = stringHashing(record.name, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "surname")){
        number_of_block = stringHashing(record.surname, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "address")){
        number_of_block = stringHashing(record.address, header_info.numBuckets) + 1;
    }

    //number_of_block = intHashing(record.id,header_info.numBuckets) + 1;
    //printf("HashFunction returned block number == %d ",number_of_block);
    // BF_ReadBlock to get the block for HashTable from disk to ram
    result_value = BF_ReadBlock(header_info.fileDesc, number_of_block, &block);
    if (result_value < 0)
    {
        BF_PrintError("Error in HT_InsertEntry at BF_ReadBlock");
        error = -1;
    }
    block_ptr = (Block *)block;

    while (block_ptr->numberOfRecords == MAX_RECORDS){
        if (block_ptr->nextBlock >= 0){
            // BF_ReadBlock to get the block for HashTable from disk to ram
            result_value = BF_ReadBlock(header_info.fileDesc, block_ptr->nextBlock, &block);
            if (result_value < 0){
                BF_PrintError("Error in HT_InsertEntry at BF_ReadBlock");
                error = -1;
            }
            
            block_ptr = (Block *)block;
        }
        
        if (block_ptr->numberOfRecords == MAX_RECORDS && block_ptr->nextBlock == -1){
            result_value = BF_AllocateBlock(header_info.fileDesc);
            if (result_value < 0)
            {
                BF_PrintError("Error in HT_InsertEntry at BF_AllocateBlock");
                error = -1;
            }
            blockCounter = BF_GetBlockCounter(header_info.fileDesc);
            if (blockCounter < 0)
            {
                BF_PrintError("Error in HT_InsertEntry at BF_GetBlockCounter");
                error = -1;
            }
            block_ptr->nextBlock = blockCounter - 1;

            memcpy(block, block_ptr, sizeof(*block_ptr));

            // BF_WriteBlock to write the block back to the file
            result_value = BF_WriteBlock(header_info.fileDesc, block_ptr->blockId);
            if (result_value < 0)
            {
                BF_PrintError("Error in HT_InsertEntry at BF_WriteBlock");
                error = -1;
            }

            result_value = BF_ReadBlock(header_info.fileDesc, blockCounter - 1, &block);
            if (result_value < 0)
            {
                BF_PrintError("Error in HT_InsertEntry at BF_ReadBlock");
                error = -1;
            }

            //initialize new block
            new_block.nextBlock = -1;
            new_block.blockId = blockCounter - 1;
            new_block.numberOfRecords = 1;

            //insert the first record in the new block
            new_block.records[0].id = record.id;
            strncpy(new_block.records[0].address, record.address, 40);
            strncpy(new_block.records[0].surname, record.surname, 20);
            strncpy(new_block.records[0].name, record.name, 15);

            memcpy(block, &new_block, sizeof(new_block));

            // BF_WriteBlock to write the block back to the file
            result_value = BF_WriteBlock(header_info.fileDesc, new_block.blockId);
            if (result_value < 0){
                BF_PrintError("Error in HT_InsertEntry at BF_WriteBlock");
                error = -1;
            }

            return new_block.blockId;
        }

    }
    //insert the record in the block
    block_ptr->records[block_ptr->numberOfRecords].id = record.id;
    strncpy(block_ptr->records[block_ptr->numberOfRecords].address, record.address, 40);
    strncpy(block_ptr->records[block_ptr->numberOfRecords].surname, record.surname, 20);
    strncpy(block_ptr->records[block_ptr->numberOfRecords].name, record.name, 15);
    block_ptr->numberOfRecords++;

    memcpy(block, block_ptr, sizeof(*block_ptr));

    // BF_WriteBlock to write the block back to the file
    result_value = BF_WriteBlock(header_info.fileDesc, block_ptr->blockId);
    if (result_value < 0)
    {
        BF_PrintError("Error in HT_InsertEntry at BF_WriteBlock");
        error = -1;
    }
    return block_ptr->blockId;

}

int HT_GetAllEntries(HT_info header_info, void *value){

    int     result_value;
    int     blocksRead = 0;
    int     numberOfRecords;
    int     flag = 0;
    int     error;    
    int     nextBlock;
    int     *val;
    char    * val1; 
    int     print = 0;
    int     number_of_block;
    void    *block;
    Block   *block_ptr;

    if (header_info.attrType == 'i'){
        val = (int *)value;
        number_of_block = intHashing(*val, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "name")){
        val1 = (char *)value;
        number_of_block = stringHashing(val1, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "surname")){
        val1 = (char *)value;
        number_of_block = stringHashing(val1, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "address")){
        val1 = (char *)value;
        number_of_block = stringHashing(val1, header_info.numBuckets) + 1;
    }    
    //the key
    //val = (int*)value;
    //int i = intHashing(*val,header_info.numBuckets) + 1; //to find the bucket

    //read the first block of the bucket which found from the hashing
    result_value = BF_ReadBlock(header_info.fileDesc, number_of_block, &block);
    if (result_value < 0){
        BF_PrintError("Error in HT_GetAllEntries at BF_ReadBlock");
        return -1;
    }
    blocksRead++;
    flag = 0;
        
    block_ptr = (Block*) block;
        
    do{ //if flag = 1 then you are not in the first loop of the do-while so you must read the next block of the bucket  
        if (flag == 1){
            result_value = BF_ReadBlock(header_info.fileDesc , nextBlock, &block);
            if (result_value < 0){
                BF_PrintError("Error in HT_GetAllEntries at BF_ReadBlock");
                return -1;
            }
            blocksRead++;
            block_ptr = (Block*) block;
        }
          
        //search in the records of this block for the key-id
        numberOfRecords = block_ptr->numberOfRecords;
        for (int j = 0 ; j < block_ptr->numberOfRecords ; j++){
            //when you find the key-id print the details
            if (header_info.attrType == 'i'){
                if (*val == block_ptr->records[j].id){
                    printf("%d %s %s %s\n",block_ptr->records[j].id,\
                    block_ptr->records[j].name,block_ptr->records[j].surname,block_ptr->records[j].address);                    
                    print = 1 ;
                }
            }
            else{
                if (!strcmp(header_info.attrName, "name") && !strcmp(val1,block_ptr->records[j].name)){
                    printf("%d %s %s %s\n", block_ptr->records[j].id,
                    block_ptr->records[j].name, block_ptr->records[j].surname, block_ptr->records[j].address);
                    print = 1;
                }
                else if (!strcmp(header_info.attrName, "surname") && !strcmp(val1, block_ptr->records[j].surname)){
                    printf("%d %s %s %s\n", block_ptr->records[j].id,
                    block_ptr->records[j].name, block_ptr->records[j].surname, block_ptr->records[j].address);
                    print = 1;
                }
                else if (!strcmp(header_info.attrName, "address") && !strcmp(val1, block_ptr->records[j].address)){
                    printf("%d %s %s %s\n", block_ptr->records[j].id,
                    block_ptr->records[j].name, block_ptr->records[j].surname, block_ptr->records[j].address);
                    print = 1;
                }
            }

        }
        nextBlock = block_ptr->nextBlock; //take the next block id to read it in the next loop 
        flag = 1;
        //printf("%d\n",block_ptr->nextBlock);
    }while(block_ptr->nextBlock != -1); //end the while when the current block has not next block
    
    //if print == 0 means that we dont found any entry so the function wasnt successful
    if (print == 0)
        return -1;
    return blocksRead;
}

int HT_DeleteEntry(HT_info header_info, void *value){

    int     result_value;
    int     error = 0;
    int     numberOfRecords;
    int     flag = 0;
    int     nextBlock;
    int     *val;
    char    *val1;
    int     number_of_block;
    void    *block;
    Block   *block_ptr;

    if (header_info.attrType == 'i'){
        val = (int *)value;
        number_of_block = intHashing(*val, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "name")){
        val1 = (char *)value;
        number_of_block = stringHashing(val1, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "surname")){
        val1 = (char *)value;
        number_of_block = stringHashing(val1, header_info.numBuckets) + 1;
    }
    else if (!strcmp(header_info.attrName, "address")){
        val1 = (char *)value;
        number_of_block = stringHashing(val1, header_info.numBuckets) + 1;
    } 
    //the key id to delete
    //val = (int*)value;
    //int i = intHashing(*val,header_info.numBuckets) + 1; //to find the bucket

    //read the first block of the bucket which found from the hashing
    result_value = BF_ReadBlock(header_info.fileDesc, number_of_block, &block);
    if (result_value < 0){
        BF_PrintError("Error in HT_DeleteEntry at BF_ReadBlock");
        error = -1;
    }
    flag = 0;
        
    block_ptr = (Block*) block;      

    do{ //if flag = 1 then you are not in the first loop of the do-while so you must read the next block of the bucket 
        if (flag == 1){
            result_value = BF_ReadBlock(header_info.fileDesc , nextBlock, &block);
            if (result_value < 0){
                BF_PrintError("Error in HT_DeleteEntry at BF_ReadBlock");
                error = -1;
            }
            block_ptr = (Block*) block;
        }
            
        //search in the records of this block for the key-id
        numberOfRecords = block_ptr->numberOfRecords;
        for (int j = 0 ; j < block_ptr->numberOfRecords ; j++){
            //when you find the key-id delete it
            if (header_info.attrType == 'i'){
                if (*val == block_ptr->records[j].id){
                    printf("The record with id %d will be deleted\n",block_ptr->records[j].id);

                    //if the deleted record is the last one didnt need to change index with the last record
                    if (j != block_ptr->numberOfRecords - 1){

                        //take the last record of the block and put it in the index of the deleted record
                        block_ptr->records[j].id = block_ptr->records[numberOfRecords-1].id;
                        strncpy(block_ptr->records[j].name, block_ptr->records[numberOfRecords-1].name , 15);
                        strncpy(block_ptr->records[j].surname, block_ptr->records[numberOfRecords-1].surname , 20);
                        strncpy(block_ptr->records[j].address, block_ptr->records[numberOfRecords-1].address , 40);
                    }

                    //initialize the last record which changed index with the deleted
                    block_ptr->records[numberOfRecords-1].id = -1 ;
                    strncpy(block_ptr->records[numberOfRecords-1].name,"\0",1);
                    strncpy(block_ptr->records[numberOfRecords-1].surname,"\0",1);
                    strncpy(block_ptr->records[numberOfRecords-1].address,"\0",1);

                    block_ptr->numberOfRecords--; 

                    memcpy(block, block_ptr, sizeof(*block_ptr));               
                    result_value = BF_WriteBlock(header_info.fileDesc, block_ptr->blockId);
                    if (result_value < 0){
                        BF_PrintError("Error in HT_DeleteEntry at BF_WriteBlock");
                        error = -1;
                    }
                    return 0;
                }
            }
            else{
                if (!strcmp(header_info.attrName, "name") && !strcmp(val1,block_ptr->records[j].name)){
                    //printf("The record with id %d will be deleted\n", block_ptr->records[j].id);

                    //if the deleted record is the last one didnt need to change index with the last record
                    if (j != block_ptr->numberOfRecords - 1)
                    {

                        //take the last record of the block and put it in the index of the deleted record
                        block_ptr->records[j].id = block_ptr->records[numberOfRecords - 1].id;
                        strncpy(block_ptr->records[j].name, block_ptr->records[numberOfRecords - 1].name, 15);
                        strncpy(block_ptr->records[j].surname, block_ptr->records[numberOfRecords - 1].surname, 20);
                        strncpy(block_ptr->records[j].address, block_ptr->records[numberOfRecords - 1].address, 40);
                    }

                    //initialize the last record which changed index with the deleted
                    block_ptr->records[numberOfRecords - 1].id = -1;
                    strncpy(block_ptr->records[numberOfRecords - 1].name, "\0", 1);
                    strncpy(block_ptr->records[numberOfRecords - 1].surname, "\0", 1);
                    strncpy(block_ptr->records[numberOfRecords - 1].address, "\0", 1);

                    block_ptr->numberOfRecords--;

                    memcpy(block, block_ptr, sizeof(*block_ptr));
                    result_value = BF_WriteBlock(header_info.fileDesc, block_ptr->blockId);
                    if (result_value < 0)
                    {
                        BF_PrintError("Error in HT_DeleteEntry at BF_WriteBlock");
                        error = -1;
                    }
                    return 0;
                }
                else if (!strcmp(header_info.attrName, "surname") && !strcmp(val1, block_ptr->records[j].surname)){
                    //printf("The record with id %d will be deleted\n", block_ptr->records[j].id);

                    //if the deleted record is the last one didnt need to change index with the last record
                    if (j != block_ptr->numberOfRecords - 1)
                    {

                        //take the last record of the block and put it in the index of the deleted record
                        block_ptr->records[j].id = block_ptr->records[numberOfRecords - 1].id;
                        strncpy(block_ptr->records[j].name, block_ptr->records[numberOfRecords - 1].name, 15);
                        strncpy(block_ptr->records[j].surname, block_ptr->records[numberOfRecords - 1].surname, 20);
                        strncpy(block_ptr->records[j].address, block_ptr->records[numberOfRecords - 1].address, 40);
                    }

                    //initialize the last record which changed index with the deleted
                    block_ptr->records[numberOfRecords - 1].id = -1;
                    strncpy(block_ptr->records[numberOfRecords - 1].name, "\0", 1);
                    strncpy(block_ptr->records[numberOfRecords - 1].surname, "\0", 1);
                    strncpy(block_ptr->records[numberOfRecords - 1].address, "\0", 1);

                    block_ptr->numberOfRecords--;

                    memcpy(block, block_ptr, sizeof(*block_ptr));
                    result_value = BF_WriteBlock(header_info.fileDesc, block_ptr->blockId);
                    if (result_value < 0)
                    {
                        BF_PrintError("Error in HT_DeleteEntry at BF_WriteBlock");
                        error = -1;
                    }
                    return 0;
                }
                else if (!strcmp(header_info.attrName, "address") && !strcmp(val1, block_ptr->records[j].address)){
                    //printf("The record with id %d will be deleted\n", block_ptr->records[j].id);

                    //if the deleted record is the last one didnt need to change index with the last record
                    if (j != block_ptr->numberOfRecords - 1)
                    {

                        //take the last record of the block and put it in the index of the deleted record
                        block_ptr->records[j].id = block_ptr->records[numberOfRecords - 1].id;
                        strncpy(block_ptr->records[j].name, block_ptr->records[numberOfRecords - 1].name, 15);
                        strncpy(block_ptr->records[j].surname, block_ptr->records[numberOfRecords - 1].surname, 20);
                        strncpy(block_ptr->records[j].address, block_ptr->records[numberOfRecords - 1].address, 40);
                    }

                    //initialize the last record which changed index with the deleted
                    block_ptr->records[numberOfRecords - 1].id = -1;
                    strncpy(block_ptr->records[numberOfRecords - 1].name, "\0", 1);
                    strncpy(block_ptr->records[numberOfRecords - 1].surname, "\0", 1);
                    strncpy(block_ptr->records[numberOfRecords - 1].address, "\0", 1);

                    block_ptr->numberOfRecords--;

                    memcpy(block, block_ptr, sizeof(*block_ptr));
                    result_value = BF_WriteBlock(header_info.fileDesc, block_ptr->blockId);
                    if (result_value < 0)
                    {
                        BF_PrintError("Error in HT_DeleteEntry at BF_WriteBlock");
                        error = -1;
                    }
                    return 0;
                }
            }    
        }
        nextBlock = block_ptr->nextBlock;
        flag = 1;
    }while(block_ptr->nextBlock != -1); //end the while when the current block has not next block
    
    printf("The record with id %d didnt found\n",*val); //didnt found the record so return failure
    return -1;
}
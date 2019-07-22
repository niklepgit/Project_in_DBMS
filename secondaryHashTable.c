#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "secondaryHashTable.h"
#include "hashFunctions.h"
#include "BF.h"
#include "blocks.h"

int SHT_SecondaryInsertEntry(SHT_info , SecondaryRecord );

int SHT_CreateSecondaryIndex(char *sfileName, char *attrName, int attrLength, int numBuckets , char* fileName){

    int result_value;
    int error = 0;
    int file_desc;
    void *Vblock;
    SecondaryBlock Secblock;
    SHT_info hash_table_info;

    // BF_CreateFile to create the file that we want
    result_value = BF_CreateFile(sfileName);
    if (result_value < 0)
    {
        BF_PrintError("Error in SHT_CreateIndex at BF_CreateFile");
        error = -1;
    }

    // BF_OpenFile to open the file
    file_desc = BF_OpenFile(sfileName);
    if (file_desc < 0)
    {
        BF_PrintError("Error in SHT_CreateIndex at BF_OpenFile");
        error = -1;
    }

    result_value = BF_AllocateBlock(file_desc);
    if (result_value < 0){
        BF_PrintError("Error in SHT_CreateIndex at BF_AllocateBlock");
        error = -1;
    }

    result_value = BF_ReadBlock(file_desc, 0, &Vblock);
    if (result_value < 0){
        BF_PrintError("Error in SHT_CreateIndex at BF_ReadBlock");
        error = -1;
    }

    // We fill the block0 with the hash_table_info
    hash_table_info.fileDesc = file_desc;
    hash_table_info.attrLength = attrLength;
    hash_table_info.numBuckets = numBuckets;

    strcpy(hash_table_info.attrName, attrName);
    strncpy(hash_table_info.fileName, fileName, 20);

    //we need memcpy to create a byteArray in order to write the block back to file on disk
    memcpy(Vblock, &hash_table_info, sizeof(hash_table_info));

    result_value = BF_WriteBlock(file_desc, 0);
    if (result_value < 0){
        BF_PrintError("Error in SHT_CreateIndex at BF_WriteBlock");
        error = -1;
    }
    // BF_AllocateBlock to allocate the block we want for the SecondaryHashTable and the Buckets
    
    Secblock.nextBlock = -1;
    Secblock.numberOfRecords = 0;

    for (int i = 1; i < numBuckets + 1; i++)
    {
        result_value = BF_AllocateBlock(file_desc);
        if (result_value < 0)
        {
            BF_PrintError("Error in SHT_CreateIndex at BF_AllocateBlock");
            error = -1;
        }
    
        // BF_ReadBlock to get the block for HashTable from disk to ram
        result_value = BF_ReadBlock(file_desc, i, &Vblock);
        if (result_value < 0)
        {
            BF_PrintError("Error in HT_CreateIndex at BF_ReadBlock");
            error = -1;
        }

        Secblock.blockId = i;
        memcpy(Vblock, &Secblock, sizeof(Secblock));
        // BF_WriteBlock to write the block back to the file
        result_value = BF_WriteBlock(file_desc, i);
        if (result_value < 0)
        {
            BF_PrintError("Error in SHT_CreateIndex at BF_WriteBlock");
            error = -1;
        }
    }

    HT_info * ht = HT_OpenIndex(fileName);
    if (ht == NULL)
    {
        BF_PrintError("Error in HT_OpenIndex");
        error = -1;
    }

    void *block;
    Block block_ptr;
    int flag  =  0;
    int  nextBlock;

    //synchronize the first hash table with the second 
    //put all the records from the first hash table to the second
    for (int i = 1 ; i < ht->numBuckets + 1 ; i++)
    {   
        //open the first hash table
        result_value = BF_ReadBlock(ht->fileDesc , i, &block);
        if (result_value < 0){
            BF_PrintError("Error in SHT_CreateIndex at BF_ReadBlock");
            return -1;
        }
        
        flag = 0;
        block_ptr = *(Block*) block;

        do{ //if flag = 1 then you are not in the first loop of the do-while so you must read the next block of the bucket  
            if (flag == 1){
                result_value = BF_ReadBlock(ht->fileDesc , nextBlock, &block);
                if (result_value < 0){
                    BF_PrintError("Error in HT_GetAllEntries at BF_ReadBlock");
                    return -1;
                }
                block_ptr = *(Block*) block;
            }

            for (int j = 0 ; j < block_ptr.numberOfRecords ; j++){
                //for each record make the secrecord and put it in the second hash table
                SecondaryRecord secRec;
                secRec.blockId = block_ptr.blockId;
                if (strcmp(attrName,"name") == 0)
                    strncpy(secRec.record.name,block_ptr.records[j].name, 15);
                else if (strcmp(attrName,"surname") == 0)
                    strncpy(secRec.record.surname ,block_ptr.records[j].surname, 20);
                else if (strcmp(attrName,"address") == 0)
                    strncpy(secRec.record.address , block_ptr.records[j].address, 40);
                //insert to second hash table
                SHT_SecondaryInsertEntry(hash_table_info,secRec);

            }   
            nextBlock = block_ptr.nextBlock; //take the next block id to read it in the next loop 
            flag = 1;

        }while(block_ptr.nextBlock != -1);
    }

    result_value = BF_CloseFile(file_desc); //close second hash table
    if (result_value < 0)
    {
        BF_PrintError("Error in SHT_CreateIndex at BF_CloseFile");
        error = -1;
    }

    return error;
}

SHT_info *SHT_OpenSecondaryIndex(char *sfileName){

    int result_value;
    int error = 0;
    int file_desc;
    void *block;
    SHT_info *hash_table_info;
    SHT_info *temp_ptr;

    // BF_OpenFile to open the file
    file_desc = BF_OpenFile(sfileName);
    if (file_desc < 0)
    {
        BF_PrintError("Error in SHT_CreateIndex at BF_OpenFile");
        error = -1;
    }

    // BF_ReadBlock to get the block for SecondaryHashTable from disk to ram
    result_value = BF_ReadBlock(file_desc, 0, &block);
    if (result_value < 0)
    {
        BF_PrintError("Error in SHT_CreateIndex at BF_ReadBlock");
        error = -1;
    }

    char attrNamePtr[40];
    strncpy(attrNamePtr, (char *)block, 40);

    if ((strcmp(attrNamePtr, "name") == 0) || (strcmp(attrNamePtr,"surname") == 0) || (strcmp(attrNamePtr,"address") == 0))
    {   //make the first block with the informations for the secondary hash table
        hash_table_info = malloc(sizeof(SHT_info));
        temp_ptr = (SHT_info *)block;
        *hash_table_info = *temp_ptr;
    }
    else{
        error = -1;
    }

    if (error == -1)
    {
        return NULL;
    }
    else
    {
        return hash_table_info;
    }
}

int SHT_CloseSecondaryIndex(SHT_info *header_info){

    int result_value;
    int error = 0;

    // BF_CloseFile to close the file that we opened with BF_OpenFile
    result_value = BF_CloseFile(header_info->fileDesc);
    if (result_value < 0)
    {
        BF_PrintError("Error in SHT_CloseIndex at BF_CloseFile");
        error = -1;
    }
    if (error != -1)
    {
        free(header_info);
        header_info = NULL;
    }

    return error;
}

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord SecRecord){

    int number_of_block;
    void *block;
    SecondaryBlock *block_ptr;
    int result_value;
    int error;
    int blockCounter;
    SecondaryBlock new_block;

    void *temp_block;
    Block *temp_block_ptr;
    
    int flag  = 0;
    // open the file that has the first hash table
    int file_desc = BF_OpenFile(header_info.fileName);
    if (file_desc < 0){
        BF_PrintError("Error in SHT_SecondaryInsertEntry at BF_OpenFile");
        error = -1;
    }
    //read the block (from the first file) that supposed to exist the first-key(id) of this secondary record
    result_value = BF_ReadBlock(file_desc, SecRecord.blockId , &temp_block);
    if (result_value < 0)
    {
        BF_PrintError("Error in SHT_SecondaryInsertEntry at BF_ReadBlock");
        error = -1;
    }
    temp_block_ptr = (Block*)temp_block;
    //search in the block and if you dont found that record exit from the function
    //to insert a secondary record must exist the original record in the block of the first file that points the field SecondaryRecord.blcokId 
    for (int j = 0 ; j < temp_block_ptr->numberOfRecords ; j++){

        if (strcmp(header_info.attrName,"name") == 0){  
            if (strcmp(SecRecord.record.name,temp_block_ptr->records[j].name) == 0)
                flag = 1;
        }
        else if (strcmp(header_info.attrName,"surname") == 0){ 
            if (strcmp(SecRecord.record.surname,temp_block_ptr->records[j].surname) == 0)
                flag = 1; 
        }
        else if (strcmp(header_info.attrName,"address") == 0){
            if (strcmp(SecRecord.record.address,temp_block_ptr->records[j].address) == 0)
                flag = 1;
        }
        if (flag == 1)
            break;
    }

    // BF_CloseFile to close the file that we opened with BF_OpenFile
    result_value = BF_CloseFile(file_desc);
    if (result_value < 0)
    {
        BF_PrintError("Error in SHT_CloseIndex at BF_CloseFile");
        error = -1;
    }

    if (flag == 0)//the record didnt found so exit
    {
        printf("The secondary record you wanted to insert not exist in block %d\n",SecRecord.blockId);
        return -1;
    }    

    //check which field is the second key and hash it
    if (strcmp(header_info.attrName,"name") == 0)
        number_of_block = stringHashing(SecRecord.record.name , header_info.numBuckets) + 1;
    else if (strcmp(header_info.attrName,"surname") == 0)
        number_of_block = stringHashing(SecRecord.record.surname , header_info.numBuckets)+1;
    else if (strcmp(header_info.attrName,"address") == 0)
        number_of_block = stringHashing(SecRecord.record.address , header_info.numBuckets)+1;

    // BF_ReadBlock to get the block for HashTable from disk to ram

    result_value = BF_ReadBlock(header_info.fileDesc, number_of_block, &block);
    if (result_value < 0)
    {
        BF_PrintError("Error in SHT_SecondaryInsertEntry at BF_ReadBlock");
        error = -1;
    }
    block_ptr = (SecondaryBlock *)block;
    //if the first block of the bucket is full go to the next one until you found a block that can hold another one secondary record
    while (block_ptr->numberOfRecords == MAX_SECONDARY_RECORDS ){
        if (block_ptr->nextBlock >= 0){
            // BF_ReadBlock to get the block for HashTable from disk to ram
            result_value = BF_ReadBlock(header_info.fileDesc, block_ptr->nextBlock, &block);
            if (result_value < 0)
            {
                BF_PrintError("Error in SHT_SecondaryInsertEntry at BF_ReadBlock");
                error = -1;
            }
            block_ptr = (SecondaryBlock*)block;

        }
        else if (block_ptr->numberOfRecords == MAX_SECONDARY_RECORDS && block_ptr->nextBlock == -1){
            //if the last block of the bucket is full allocate a new next block
            result_value = BF_AllocateBlock(header_info.fileDesc);
            if (result_value < 0)
            {
                BF_PrintError("Error in HT_CreateIndex at BF_AllocateBlock");
                error = -1;
            }
            blockCounter = BF_GetBlockCounter(header_info.fileDesc);
            if (blockCounter < 0)
            {
                BF_PrintError("Error in SHT_SecondaryInsertEntry at BF_GetBlockCounter");
                error = -1;
            }
            block_ptr->nextBlock = blockCounter - 1; //put the id of the new next block in the previous block
            memcpy(block,block_ptr,sizeof(*block_ptr));

            // BF_WriteBlock to write the block back to the file
            result_value = BF_WriteBlock(header_info.fileDesc, block_ptr->blockId);
            if (result_value < 0)
            {
                BF_PrintError("Error in HT_CreateIndex at BF_WriteBlock");
                error = -1;
            }

            result_value = BF_ReadBlock(header_info.fileDesc, blockCounter - 1, &block);
            if (result_value < 0)
            {
                BF_PrintError("Error in SHT_SecondaryInsertEntry at BF_ReadBlock");
                error = -1;
            }
            //initialize block
            new_block.nextBlock = -1;
            new_block.blockId = blockCounter - 1;

            //the block from the first file that has the original record
            new_block.records[0].blockId = SecRecord.blockId;
            //the first secondary record of the block
            if (strcmp(header_info.attrName,"name") == 0)
                strncpy(new_block.records[0].record.name, SecRecord.record.name, 15);
            else if (strcmp(header_info.attrName,"surname") == 0)
                strncpy(new_block.records[0].record.surname, SecRecord.record.surname, 20);
            else if (strcmp(header_info.attrName,"address") == 0)
                strncpy(new_block.records[0].record.address, SecRecord.record.address, 40);

            
            new_block.numberOfRecords = 1;
            memcpy(block, &new_block, sizeof(new_block));

            // BF_WriteBlock to write the block back to the file
            result_value = BF_WriteBlock(header_info.fileDesc, new_block.blockId);
            if (result_value < 0)
            {
                BF_PrintError("Error in SHT_SecondaryInsertEntry at BF_WriteBlock");
                error = -1;
            }
            return 0;
        }
    }

    //put a secondary record in the block
    if (strcmp(header_info.attrName,"name") == 0)
        strncpy(block_ptr->records[block_ptr->numberOfRecords].record.name, SecRecord.record.name, 15);
    else if (strcmp(header_info.attrName,"surname") == 0)    
        strncpy(block_ptr->records[block_ptr->numberOfRecords].record.surname, SecRecord.record.surname, 20);
    else if (strcmp(header_info.attrName,"address") == 0)    
        strncpy(block_ptr->records[block_ptr->numberOfRecords].record.address, SecRecord.record.address, 40);

    block_ptr->records[block_ptr->numberOfRecords].blockId = SecRecord.blockId;
    block_ptr->numberOfRecords++;

    memcpy(block, block_ptr, sizeof(*block_ptr));

    // BF_WriteBlock to write the block back to the file
    result_value = BF_WriteBlock(header_info.fileDesc, block_ptr->blockId);
    if (result_value < 0)
    {
        BF_PrintError("Error in SHT_SecondaryInsertEntry at BF_WriteBlock");
        error = -1;
    }

    return 0;

}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value){

    int     result_value;
    int     blocksRead = 0;
    int     numberOfRecords;
    int     flag = 0;
    int     print = 0;
    int     error;    
    int     nextBlock;
    char     val[40];
    void    *SecBlock;
    SecondaryBlock   *Sec_block_ptr;

    void    *block;
    Block   *block_ptr;    

    int     allocatedBlocks = BF_GetBlockCounter(header_info_ht.fileDesc);
    int     *Visited_Block_IDs;
    int     index = 0;

    Visited_Block_IDs = malloc(allocatedBlocks * sizeof(int)); //an array to keep the ids of the visited blocks
    if (Visited_Block_IDs == NULL)
    {
        printf("Error in malloc at function SHT_SecondaryGetAllEntries\n");
        return -1;
    }

    //initialize array
    for (int i = 0 ; i < allocatedBlocks ; i++)
        Visited_Block_IDs[i] = -2;

    //the key
    strncpy(val,(char*)value,40);
    int i = stringHashing(val , header_info_sht.numBuckets) + 1; //hash to find the bucket of the value

    result_value = BF_ReadBlock(header_info_sht.fileDesc , i, &SecBlock);
    if (result_value < 0){
        BF_PrintError("Error in SHT_SecondaryGetAllEntries11 at BF_ReadBlock");
        return -1;
    }
    blocksRead++;
    flag = 0;

    Sec_block_ptr = (SecondaryBlock*) SecBlock;
    
    do{ //if flag = 1 then you are not in the first loop of the do-while so you must read the next block of the bucket  
        if (flag == 1){
            result_value = BF_ReadBlock(header_info_sht.fileDesc , nextBlock, &SecBlock);
            if (result_value < 0){
                BF_PrintError("Error in SHT_SecondaryGetAllEntries12 at BF_ReadBlock");
                return -1;
            }
            blocksRead++;
            Sec_block_ptr = (SecondaryBlock*) SecBlock;
        }
            
        //search in the records of this block for the secondary key
        numberOfRecords = Sec_block_ptr->numberOfRecords;
        for (int j = 0 ; j < Sec_block_ptr->numberOfRecords ; j++){
                
            int found = 0 ;
            if (strcmp(header_info_sht.attrName,"name") == 0){  
                if (strcmp(val, Sec_block_ptr->records[j].record.name) == 0)
                    found = 1;
            }
            else if (strcmp(header_info_sht.attrName,"surname") == 0){
                if (strcmp(val, Sec_block_ptr->records[j].record.surname) == 0)
                    found = 1;
            }
            else if (strcmp(header_info_sht.attrName,"address") == 0){
                if (strcmp(val, Sec_block_ptr->records[j].record.address) == 0)
                    found = 1;
            }

            if (found == 1)//found the secondary record that has the secondary key
            {   
                //we keep an array of the block ids that visit in the primary file
                //because each block id we visit we print all the records that have the secondary key
                //so we must not visit it again if we found another secondary record which points to this block id
                int flag2 = 0; 
                for(int i = 0 ; i < allocatedBlocks ; i++)
                    if (Visited_Block_IDs[i] == Sec_block_ptr->blockId)
                        flag2 = 1;
                    
                if (flag2 == 0)//the block id of the primary file has not visited so read it
                {    
                    result_value = BF_ReadBlock(header_info_ht.fileDesc , Sec_block_ptr->records[j].blockId, &block);
                    if (result_value < 0){
                        BF_PrintError("Error in SHT_SecondaryGetAllEntries at BF_ReadBlock");
                        return -1;
                    }
                    blocksRead++;
                    block_ptr = (Block*) block;
                    
                    //search the block to find the record with the secondary key
                    for (int k = 0 ; k < block_ptr->numberOfRecords ; k++)
                    {   
                        if (strcmp(header_info_sht.attrName,"name") == 0){  
                            if (strcmp(val, block_ptr->records[k].name) == 0)
                            {   printf("%d %s %s %s\n",block_ptr->records[k].id,\
                                block_ptr->records[k].name,block_ptr->records[k].surname,block_ptr->records[k].address); 
                                
                                print = 1;
                            }
                        }
                        else if (strcmp(header_info_sht.attrName,"surname") == 0){
                            if (strcmp(val, block_ptr->records[k].surname) == 0)
                            {   printf("%d %s %s %s\n",block_ptr->records[k].id,\
                                block_ptr->records[k].name,block_ptr->records[k].surname,block_ptr->records[k].address); 
                                
                                print = 1;
                            }
                        }
                        else if (strcmp(header_info_sht.attrName,"address") == 0){
                            if (strcmp(val, block_ptr->records[k].address) == 0)
                            {   printf("%d %s %s %s\n",block_ptr->records[k].id,\
                                block_ptr->records[k].name,block_ptr->records[k].surname,block_ptr->records[k].address); 
                            
                                print = 1;
                            }
                        }
                    }

                    Visited_Block_IDs[index] = Sec_block_ptr->records[j].blockId; //put the block id in the visited block ids
                    index++;
                }
            }
        }
        nextBlock = Sec_block_ptr->nextBlock; //take the next block id to read it in the next loop 
        flag = 1;
    }while(Sec_block_ptr->nextBlock != -1); //end the while when the current block has not next block
    
    free(Visited_Block_IDs);
    
    //if print == 0 means that we dont found any entry so the function wasnt successful
    if (print == 0)
        return -1;

    return blocksRead;
}

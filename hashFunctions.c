#include "hashFunctions.h"
#include <string.h>
#include <stdio.h>
#include "BF.h"
#include "blocks.h"
#include "records.h"
#include "hashTable.h"
#include "secondaryHashTable.h"

int HashStatistics(char* fileName)
{
    int result_value;
    int buckets;
    int file_desc;
    int nextBlock;
    HT_info *ht_info;
    SHT_info * sht_info;
    
    ht_info = HT_OpenIndex(fileName);

    //check if the file is hash table or secondary hash table
    if ( ht_info != NULL)
    {   
        printf("\nHash Table %s Statistics:\n",fileName);
        buckets = ht_info->numBuckets;
        file_desc = ht_info->fileDesc;

        int numberOfBlocks = BF_GetBlockCounter(file_desc);
        printf("Blocks Allocated: %d\n" , numberOfBlocks);

        void * block;
        Block *block_ptr;
        int totalRecords = 0 ;
        int min , max  ;
        int maxIndex , minIndex;
        for (int i = 1 ; i < buckets + 1 ; i++)
        {   //read the first block of each bucket
            result_value = BF_ReadBlock(file_desc , i , &block);
            if (result_value < 0){
                BF_PrintError("Error in HashStatistics at BF_ReadBlock");
                return -1;
            }
            int BucketRecords = 0 ;
            int overflow = 0;

            int flag = 0;
            block_ptr = (Block*) block;
        
            do{ //if flag = 1 then you are not in the first loop of the do-while so you must read the next block of the bucket  
                if (flag == 1){
                    result_value = BF_ReadBlock(file_desc , nextBlock, &block);
                    if (result_value < 0){
                        BF_PrintError("Error in HashStatistics at BF_ReadBlock");
                        return -1;
                    }
                    block_ptr = (Block*) block;
                    overflow++; //count the overflow blocks of the bucket
                }
                BucketRecords = BucketRecords + block_ptr->numberOfRecords;

                nextBlock = block_ptr->nextBlock; //take the next block id to read it in the next loop 
                flag = 1;
            }while(block_ptr->nextBlock != -1); //end the while when the current block has not next block

            if (overflow == 0)
                printf("Bucket %d has not overflow block\n",i);
            else
                printf("Bucket %d has overflow %d blocks\n",i,overflow);
            
            totalRecords = totalRecords + BucketRecords;
            if (i == 1)
            {
                min = BucketRecords; //initialize values
                max = BucketRecords;
                minIndex = i;
                maxIndex = i;
                continue;
            }
            if (max < BucketRecords)
            {   max = BucketRecords;    //keep the bucket with the most records
                maxIndex = i ;  
            }
            if (min > BucketRecords)
            {   min = BucketRecords;    //keep the bucket with the least records
                minIndex = i ;
            }
        }

        printf("Bucket %d has the most records: %d\n",maxIndex,max);
        printf("Bucket %d has the least records:    %d\n",minIndex,min);;
        printf("Average records per bucket: %d\n",totalRecords/buckets);
        printf("Average blocks per bucket:  %d\n",numberOfBlocks/buckets);
    }
    else
    {
        printf("\nSecondary Hash Table %s Statistics:\n",fileName );
        sht_info = SHT_OpenSecondaryIndex(fileName);
        buckets = sht_info->numBuckets;
        file_desc = sht_info->fileDesc;

        int numberOfBlocks = BF_GetBlockCounter(file_desc);
        printf("Blocks Allocated: %d\n" , numberOfBlocks);

        void * block;
        SecondaryBlock *block_ptr;
        int totalRecords = 0 ;
        int min , max ;
        int maxIndex , minIndex;
        for (int i = 1 ; i < buckets + 1 ; i++)
        {   //read the first block of each bucket
            result_value = BF_ReadBlock(file_desc , i , &block);
            if (result_value < 0){
                BF_PrintError("Error in HashStatistics at BF_ReadBlock");
                return -1;
            }
            int BucketRecords = 0 ;
            int overflow = 0;

            int flag = 0;
            block_ptr = (SecondaryBlock*) block;
        
            do{ //if flag = 1 then you are not in the first loop of the do-while so you must read the next block of the bucket  
                if (flag == 1){
                    result_value = BF_ReadBlock(file_desc , nextBlock, &block);
                    if (result_value < 0){
                        BF_PrintError("Error in HashStatistics at BF_ReadBlock");
                        return -1;
                    }
                    block_ptr = (SecondaryBlock*) block;
                    overflow++;  //count the overflow blocks of the bucket
                }
                BucketRecords = BucketRecords + block_ptr->numberOfRecords;

                nextBlock = block_ptr->nextBlock; //take the next block id to read it in the next loop 
                flag = 1;
            }while(block_ptr->nextBlock != -1); //end the while when the current block has not next block

            if (overflow == 0)
                printf("Bucket %d has not overflow block\n",i);
            else
                printf("Bucket %d has overflow %d blocks\n",i,overflow);
            
            totalRecords = totalRecords + BucketRecords;
            if (i == 1)
            {
                min = BucketRecords; //initialize values
                max = BucketRecords;
                minIndex = i;
                maxIndex = i;
                continue;
            }
            if (max < BucketRecords)
            {   max = BucketRecords;    //keep the bucket with the most records
                maxIndex = i ;  
            }
            if (min > BucketRecords)
            {   min = BucketRecords;    //keep the bucket with the least records
                minIndex = i ;
            }
        }

        printf("Bucket %d has the most records: %d\n",maxIndex,max);
        printf("Bucket %d has the least records:    %d\n",minIndex,min);;
        printf("Average records per bucket: %d\n",totalRecords/buckets);
        printf("Average blocks per bucket:  %d\n",numberOfBlocks/buckets);      
    }
    return 0;
}


// We will use Universal Hashing for integers and strings

int intHashing(int key,int buckets){
    int a      = 24;
    int b      = 16;
    int prime  = 2617;

    return ((((long)(a*key)+b) % prime) % buckets);
}

int stringHashing(char* key , int buckets){
    int a     = 4;
    int prime = 421;
    long long h     = 0;

    for (int i = 0; i < strlen(key); i++){
        h = ((h*a)+key[i]);
        //printf(" char %c,",key[i]);
    }
        //printf("h = %ld, prime = %ld, number of buckets = %d,\n",h,prime,NUMBER_OF_BUCKETS);
    return ((h % prime) % buckets);
}
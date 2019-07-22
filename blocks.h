#ifndef __BLOCK__
#define __BLOCK__

#define MAX_RECORDS ((512 - 4 - 4 - 4 ) / sizeof(Record)) /*the number of how many records can fit in a block*/
#define MAX_SECONDARY_RECORDS ((512 - 4 - 4 - 4) / sizeof(SecondaryRecord)) /*the number of how many records can fit in a block*/
#include "records.h"

typedef struct Block
{
    int blockId;
    Record records[MAX_RECORDS];
    int numberOfRecords;
    int nextBlock; /*next block id*/

} Block;

typedef struct SecondaryBlock
{
    int blockId;
    SecondaryRecord records[MAX_SECONDARY_RECORDS];
    int numberOfRecords;
    int nextBlock;
    
} SecondaryBlock;

#endif //end of __BLOCK__
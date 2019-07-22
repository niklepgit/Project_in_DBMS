#ifndef __RECORDS__
#define __RECORDS__

typedef struct record {

	int id ;
	char name[15];
	char surname[20];
	char address[40];

} Record;

typedef struct SecondaryRecord{

	Record record;
	int blockId;

}SecondaryRecord;

#endif //end of __RECORDS__
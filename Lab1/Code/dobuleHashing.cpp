#include "readfile.h"


#define a1 3
#define b1 7
/* Hash function to choose bucket
 * Input: key used to calculate the hash
 * Output: HashValue;
 */
int hashCodeFirst(int key){
   return key % MBUCKETS;
}

int hashCodeSecond(int key,int firstHash){
   return abs(firstHash - ((a1*key+b1) % MBUCKETS));
}





int insertItem(int fd,DataItem item){
   //Definitions
	struct DataItem data;   //a variable to read in it the records from the db
	int count = 0;				//No of accessed records
	int rewind = 0;			//A flag to start searching from the first bucket
	int hashIndex = hashCodeFirst(item.key);  				//calculate the Bucket index
	int startingOffset = hashIndex*sizeof(Bucket);		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db
    bool secondHash = false;

	//Main Loop
	RESEEK:
	//on the linux terminal use man pread to check the function manual
	ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
	//one record accessed
	count++;
	//check whether it is a valid record or not
    if(result <= 0) //either an error happened in the pread or it hit an unallocated space
	{ 	 // perror("some error occurred in pread");
		  return -1;
    }
    else if (data.valid == 0 ) { //empty space
    	//I found the empty record
    			pwrite(fd,&item,sizeof(DataItem), Offset);
                return count;

    } else { //not the empty record I am looking for
            if(secondHash){// using openAddressing after second Hashing
                Offset +=sizeof(DataItem);  //move the offset to next record
                if(Offset >= FILESIZE && rewind ==0 )
                { //if reached end of the file start again
                        rewind = 1;
                        Offset = 0;
                        goto RESEEK;
                } else
                    if(rewind == 1 && Offset >= startingOffset) {
                        return -1; //no empty spaces
                }
                goto RESEEK;

            }
            else{
				//Search Inside Bucket 
				for(int i = 1 ; i<RECORDSPERBUCKET;i++){
					Offset +=sizeof(DataItem);
					count ++;
					ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
					if(result <= 0) {
						return -1;
					}
					else if (data.valid == 0 ) 
					{ 
								pwrite(fd,&item,sizeof(DataItem), Offset);
								return count;

					}	
				}
				//Didnt Find empty space inside bucket so will do second hash 
                 hashIndex = hashCodeSecond(item.key,hashIndex);  				
	             startingOffset = hashIndex*sizeof(Bucket);
            	 Offset = startingOffset;	
                 secondHash = true;
                 goto RESEEK;

            }
    		
    }
   return count;
}


/* Functionality: using a key, it searches for the data item
 *          1. use the hash function to determine which bucket to search into
 *          2. search for the element starting from this bucket and till it find it.
 *          3. return the number of records accessed (searched)
 *
 * Input:  fd: filehandler which contains the db
 *         item: the dataitem which contains the key you will search for
 *               the dataitem is modified with the data when found
 *         count: No. of record searched
 *
 * Output: the in the file where we found the item
 */

int searchItem(int fd,struct DataItem* item,int *count)
{

	//Definitions
	struct DataItem data;   //a variable to read in it the records from the db
	*count = 0;				//No of accessed records
	int rewind = 0;			//A flag to start searching from the first bucket
	int hashIndex = hashCodeFirst(item->key);  				//calculate the Bucket index
	int startingOffset = hashIndex*sizeof(Bucket);		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db
    bool secondHash = false;

	//Main Loop
	RESEEK:
	//on the linux terminal use man pread to check the function manual
	ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
	//one record accessed
	(*count)++;
	//check whether it is a valid record or not
    if(result <= 0) //either an error happened in the pread or it hit an unallocated space
	{ 	 // perror("some error occurred in pread");
		  return -1;
    }
    else if (data.valid == 1 && data.key == item->key) {
    	//I found the needed record
    			item->data = data.data ;
    			return Offset;

    } else { //not the record I am looking for
            if(secondHash){
                Offset +=sizeof(DataItem);  //move the offset to next record
                if(Offset >= FILESIZE && rewind ==0 )
                { //if reached end of the file start again
                        rewind = 1;
                        Offset = 0;
                        goto RESEEK;
                } else
                    if(rewind == 1 && Offset >= startingOffset) {
                        return -1; //no empty spaces
                }
                goto RESEEK;

            }
            else{
				for(int i = 1 ; i<RECORDSPERBUCKET;i++){
					Offset +=sizeof(DataItem);
					count ++;
					ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
					if(result <= 0) {
						return -1;
					}
					else if (data.valid == 1 && data.key == item->key ) 
					{ 
								item->data = data.data ;
								return Offset;

					}	
				}

                secondHash = true;
                 hashIndex = hashCodeSecond(item->key,hashIndex);  				
	             startingOffset = hashIndex*sizeof(Bucket);
            	 Offset = startingOffset;	
                 secondHash = true;
                 goto RESEEK;

            }
    		
    }
}


/* Functionality: Display all the file contents
 *
 * Input:  fd: filehandler which contains the db
 *
 * Output: no. of non-empty records
 */
int DisplayFile(int fd){

	struct DataItem data;
	int count = 0;
	int Offset = 0;
	for(Offset =0; Offset< FILESIZE;Offset += sizeof(DataItem))
	{
		ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
		if(result < 0)
		{ 	  perror("some error occurred in pread");
			  return -1;
		} else if (result == 0 || data.valid == 0 ) { //empty space found or end of file
			printf("Bucket: %d, Offset %d:~\n",Offset/BUCKETSIZE,Offset);
		} else {
			pread(fd,&data,sizeof(DataItem), Offset);
			printf("Bucket: %d, Offset: %d, Data: %d, key: %d\n",Offset/BUCKETSIZE,Offset,data.data,data.key);
					 count++;
		}
	}
	return count;
}


/* Functionality: Delete item at certain offset
 *
 * Input:  fd: filehandler which contains the db
 *         Offset: place where it should delete
 *
 * Hint: you could only set the valid key and write just and integer instead of the whole record
 */
int deleteOffset(int fd, int Offset)
{
	struct DataItem dummyItem;
	dummyItem.valid = 0;
	dummyItem.key = -1;
	dummyItem.data = 0;
	int result = pwrite(fd,&dummyItem,sizeof(DataItem), Offset);
	return result;
}


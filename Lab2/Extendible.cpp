#include "readfile.h"

/**
 * @brief Search for an item given its key, and the offset of the bucket that contains it
 * 
 * @param fd Buckets file's discriptor
 * @param offset Bucket's offset
 * @param key Item's key
 * @return int -1 if not found, 1 if found
 */
int searchItem(int fd,int offset,int key)
{
	Bucket bucket;
	pread(fd, &bucket, sizeof(Bucket), offset);
	for(int i = 0; i < RECORDSPERBUCKET; i++)
	{
		if(bucket.data[i].valid && bucket.data[i].key == key)	// found
		{
			printf("%d found\n", key);
			return 1;
		}
	}
	printf("%d not found\n", key);
	return -1;
}

/**
 * @brief Insert an item in a bucket given its offset
 * 
 * @param fd Buckets file's discriptor
 * @param offset Bucket's offset
 * @param item Item
 * @return int -1 if an empty place found, local depth if no empty place found
 */
int insertItem(int fd,int offset,DataItem item){
   	Bucket bucket;
   	pread(fd,&bucket,sizeof(Bucket),offset);	//fetch bucket from disk
	
   	bool found = false;
   	for(int i=0; i < RECORDSPERBUCKET; i++)
   	{
		if(bucket.data[i].valid && bucket.data[i].key == item.key) //exists
		{
			printf("%d exists\n");
			return -1;
		}
		if(bucket.data[i].valid == 0)	//empty position, insert data
		{
			bucket.data[i] = item;
			found = true;
			break;
		}
   	}
   	pwrite(fd,&bucket,sizeof(Bucket),offset);
   	return found ? -1 : bucket.depth;
}

/**
 * @brief Delete an item given its key, and the offset of the bucket contains it
 * 
 * @param fd Buckets file's discriptor
 * @param offset Bucket's offset
 * @param key Item's key
 * @return int -1 if not found, local depth if deleted succesfully
 */
int deleteItem(int fd,int offset,int key)
{
	Bucket bucket;
	pread(fd, &bucket, sizeof(Bucket), offset);
	
	int count = 0;
	bool found = false;
	for(int i = 0; i < RECORDSPERBUCKET; i++)
	{
		if(bucket.data[i].valid)
		{
			if(bucket.data[i].key == key)	//found
			{
				bucket.data[i].valid = 0;
				found = true;
			}
			else 
				count++;
		}
	}

	if(!found)
	{
		printf("%d not found\n", key);
		return -1;
	}

	pwrite(fd, &bucket, sizeof(Bucket), offset);
	printf("%d deleted\n", key);
	return bucket.depth;
}
#include "readfile.h"

/**
 * @brief Converting a number to its binary representation 
 * 
 * @param number 
 * @return string Binary number
 */
string To_Binary(int number)
{
	string result = "";
	while(number)
	{
		result += (char)(number % 2 + '0');
		number /= 2;
	}

	while(result.length() < LEN)
		result += "0";

	reverse(result.begin(),result.end());
	return result;
}

/**
 * @brief Hashing a key and returning its left most depth bits
 * 
 * @param key 
 * @param depth 
 * @return int Index of key
 */
int hashCode(int key,int depth){
	int temp = key % MOD;
	string binary = To_Binary(temp);

	int index = 0;
	for(int i = 0; i < depth; i++)
		index |= (binary[i]-'0') * (1 << (depth-i-1));

    return index;
}

/**
 * @brief Create a File object and return its discriptor
 * 
 * @param size
 * @param name 
 * @return int File discriptor
 */
int createFile(int size,char * name)
{
    int fd;
	int result;
    struct stat sb;

    if (stat(name, &sb) == -1)
        printf("file doesn't exist, create it\n");

    fd = open(name, O_RDWR | O_CREAT , (mode_t)0600);
    if (fd == -1) {
        perror("Error opening file for writing");
            return -1;
    }

    // expanding the file size
    result = lseek(fd, size, SEEK_SET);
    if (result == -1) {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
            return -1;
    }
    return fd;
}

/**
 * @brief Doubling the size of the pointers' table
 * 
 * @param fd File discriptor
 * @param FileSize 
 * @param oldName File's old name
 * @param name File's new name
 * @param modifiedBucket Index of last modified bucket
 * @param newBucketAddress Index of last added bucket
 * @return int File discriptor of the new file
 */
int extendTable(int fd,int FileSize,string oldName,string name,int modifiedBucket,int newBucketAddress)
{
    char * fileName = &name[0];
    char * oldFile = &oldName[0];
    int newFile = createFile(2*FileSize,fileName);

    int output_offset = 0;
    for(int offset = 0; offset < FileSize; offset += sizeof(int))
    {
        int index = offset / sizeof(int);
        if(index != modifiedBucket)
        {
            int address;
            pread(fd, &address, sizeof(int), offset);
            for(int j=0;j<2;j++)
            {
                pwrite(newFile, &address, sizeof(int),output_offset);
                output_offset += sizeof(int);
            }
        }
        else
        {
            int address;
            pread(fd, &address, sizeof(int), offset);
            pwrite(newFile, &address, sizeof(int),output_offset);
            output_offset += sizeof(int);
            pwrite(newFile, &newBucketAddress, sizeof(int),output_offset);
            output_offset += sizeof(int);
        }
    }
    remove(oldFile);
    return newFile;
}

/**
 * @brief Modifying pointers in the table without extending
 * 
 * @param fd File discriptor
 * @param index Index of the last modified bucket
 * @param depth Local depth of the last modified bucket
 * @param GDepth Global depth of table
 * @param offset Address of last added bucket
 */
void modifyTable(int fd,int index,int depth,int GDepth,int offset)
{
    string compare = To_Binary(index).substr(LEN-GDepth).substr(0,depth); 

    for(int i=0;i<(1 << GDepth);i++)
    {
        string binary = To_Binary(i).substr(LEN-GDepth).substr(0,depth+1);

        if(compare == binary.substr(0,depth) && binary[depth] == '1')
            pwrite(fd,&offset,sizeof(int),i * sizeof(int));
    }
}

/**
 * @brief Shrinking table size
 * 
 * @param td Table's file discriptor
 * @param fd Buckets' file discriptor
 * @param FILESIZE Buckets' file size
 * @param GlobalDepth Table's global depth
 * @param oldName Table file's old name
 * @param newName Table file's new name
 * @return int New table file's discriptor
 */
int Shrink(int td, int fd, int FILESIZE, int  &GlobalDepth, string oldName, string &newName)
{
    int maxDepth = 0;
    for(int offset = 0; offset < FILESIZE; offset += sizeof(Bucket))
    {
        Bucket bucket;
        pread(fd, &bucket, sizeof(Bucket), offset);

        if(bucket.valid == 0)
            maxDepth = max(maxDepth, bucket.depth);
    }

    if(maxDepth < GlobalDepth)
    {   
        td = ShrinkTable(td, (1 << GlobalDepth) * sizeof(int), 1 << (GlobalDepth - maxDepth), oldName, newName + "0");
        newName += "0";
        GlobalDepth = maxDepth;
    }
    return td;
}

/**
 * @brief Utility functions used to create the new table with a new size
 * 
 * @param fd Table file's discriptor
 * @param FileSize Table file's size
 * @param factor Shrinking factor
 * @param oldName Table file's old name
 * @param name Table file's new name
 * @return int New table file's discriptor
 */
int ShrinkTable(int fd,int FileSize,int factor,string oldName,string name)
{
    char * fileName = &name[0];
    char * oldFile = &oldName[0];
    int newFile = createFile(FileSize/factor,fileName);

    int output_offset = 0;
    for(int offset = 0; offset < FileSize; offset += factor * sizeof(int))
    {
        int address;
        pread(fd, &address, sizeof(int), offset);

        pwrite(newFile, &address, sizeof(int),output_offset);
        output_offset += sizeof(int);
    }
    remove(oldFile);
    return newFile;
}

/**
 * @brief Initiate a bucket with a depth
 * 
 * @param bucket 
 * @param depth 
 */
void resetBucket(Bucket & bucket, int depth)
{
    bucket.depth = depth;
    bucket.valid = 0;
    for(int i=0;i<RECORDSPERBUCKET;i++)
        bucket.data[i].valid = bucket.data[i].data = bucket.data[i].key = 0;
}

/**
 * @brief Add a bucket to the end of the buckets' size
 * 
 * @param fd File discriptor
 * @param fileSize 
 * @param depth Local depth
 * @return int New File's size
 */
int AddBucket(int fd,int fileSize,int depth)
{
    int result;
    result = lseek(fd, fileSize, SEEK_SET);
    if (result < 0) {
        close(fd);
        perror("Error expanding the file");
            return -1;
    }
    Bucket bucket;
    resetBucket(bucket, depth);
    pwrite(fd, &bucket, sizeof(Bucket), fileSize);
    return fileSize + sizeof(Bucket);
}

/**
 * @brief Find the offset of the first empty bucket
 * 
 * @param fd File discriptor
 * @param FILESIZE 
 * @param depth 
 * @return int Bucket's offset or INT_MAX if no empty buckets exist
 */
int findBucket(int fd, int FILESIZE, int depth)
{
    for(int offset = 0; offset < FILESIZE; offset+= sizeof(Bucket))
    {
        Bucket bucket;
        pread(fd, &bucket, sizeof(Bucket), offset);
        
        if(bucket.valid)
        {
            resetBucket(bucket, depth);
            pwrite(fd, &bucket, sizeof(Bucket), offset);
            return offset;
        }
    }
    return INT_MAX;
}

/**
 * @brief Counts the number of non empty records in a bucket
 * 
 * @param b Bucket
 * @return int Number of non empty records
 */
int countRecords(const Bucket & b)
{
    int count = 0;
    for(int i = 0; i < RECORDSPERBUCKET; i++)
        if(b.data[i].valid)
            count++;

    return count;
}

/**
 * @brief Reordering pointers after deletion
 * 
 * @param td Table file's discriptor
 * @param fd Buckets file's discriptor
 * @param index Index of last modified bucket
 * @param sib Index of sibling of last modified bucket
 * @param base Local part in the index's binary represetation
 * @param diff Difference between global depth and maximum local depth
 * @return true When merging occurs
 * @return false When merging doesn't occur
 */
bool mergeBuckets(int td,int fd,int index,int sib,int base, int diff)
{
    Bucket b1,b2;
    int offset1,offset2;
    pread(td, &offset1, sizeof(int), index * sizeof(int));
    pread(td, &offset2, sizeof(int), sib * sizeof(int));
    pread(fd, &b1, sizeof(Bucket), offset1);
    pread(fd, &b2, sizeof(Bucket), offset2);

    int count1 = countRecords(b1), count2 = countRecords(b2);
    if(b1.depth == b2.depth && offset1 != offset2 && count1 + count2 <= RECORDSPERBUCKET)   //merge
    {
        int j = 0;
        while(j < RECORDSPERBUCKET && b1.data[j].valid)
            j++;
        for(int i=0;i<RECORDSPERBUCKET;i++)
        {
            if(b2.data[i].valid)
            {
                b1.data[j] = b2.data[i];
                b2.data[i].valid = 0;
                while(j < RECORDSPERBUCKET && b1.data[j].valid)
                    j++;
            }
        }
        b1.depth--;
        b2.valid = 1;
        pwrite(fd, &b1, sizeof(Bucket), offset1);
        pwrite(fd, &b2, sizeof(Bucket), offset2);
        while((sib >> diff) == base)
        {
            pwrite(td, &offset1, sizeof(int), sib * sizeof(int));
            sib++;
        }
        return true;
    }
    return false;
}

/**
 * @brief Redistribute data between the old and the new added bucket based on the new local depth
 * 
 * @param fd Bucket file's discriptor
 * @param offset1 Offset of the old bucket
 * @param offset2 Offset of the new bucket
 */
void reDistribute(int fd,int offset1,int offset2)
{
	Bucket b1,b2;
	pread(fd,&b1,sizeof(Bucket),offset1);
	pread(fd,&b2,sizeof(Bucket),offset2);
	b1.depth++;
	b2.depth++;
	int j=0;
	for(int i=0;i<RECORDSPERBUCKET;i++)
	{
		int key = b1.data[i].key % MOD;
		string binary = To_Binary(key).substr(0,b1.depth);
		if(binary[binary.length()-1] == '1')
		{
			b2.data[j++] = b1.data[i];
			b1.data[i].valid = 0;
		}
	}
	pwrite(fd,&b1,sizeof(Bucket),offset1);
	pwrite(fd,&b2,sizeof(Bucket),offset2);
}

/**
 * @brief Display the contents of the Table
 * 
 * @param fd Table file's discriptor
 * @param FILESIZE Table file's size in bytes
 */
void DisplayTable(int fd, int FILESIZE){
	int data;
    printf("-------------------------------------\n");
	int Offset = 0;
	for(Offset =0; Offset< FILESIZE;Offset += sizeof(int))
	{
		pread(fd,&data,sizeof(int), Offset);

        printf("pointer: %d, Offset: %d, pointing to: %d\n",Offset/sizeof(int),Offset,data);
        printf("-------------------------------------\n");
	}
}

/**
 * @brief Display the contents of the Buckets
 * 
 * @param fd Buckets file's discriptor
 * @param FILESIZE Buckets file's size
 */
void DisplayFile(int fd, int FILESIZE){
	Bucket data;
    printf("-------------------------------------\n");
	int Offset = 0;
	for(Offset =0; Offset< FILESIZE;Offset += sizeof(Bucket))
	{
		pread(fd,&data,sizeof(Bucket), Offset);
        printf("Bucket at Offset: %d, with depth: %d, validity: %d\n",Offset,data.depth,data.valid);
        for(int i=0;i<RECORDSPERBUCKET;i++)
        {
            if (data.data[i].valid && data.valid == 0)
                printf("Data: %d, key: %d, valid:%d\n",data.data[i].data,data.data[i].key,data.data[i].valid);
        }
        printf("-------------------------------------\n");
	}
}
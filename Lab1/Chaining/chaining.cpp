#include "readfile.h"

/* Hash function to choose bucket
 * Input: key used to calculate the hash
 * Output: HashValue;
 */
int hashCode(int key)
{
    return key % MBUCKETS;
}

/* Functionality insert the data item into the correct position
 *          1. use the hash function to determine which bucket to insert into
 *          2. search for the first empty space within the bucket
 *          	2.1. if it has empty space
 *          	           insert the item
 *          	     else
 *          	          use Chaining to insert the record
 *          3. return the number of records accessed (searched)
 *
 * Input:  fd: filehandler which contains the db
 *         item: the dataitem which should be inserted into the database
 *
 * Output: No. of record searched
 *
 * Hint: You can check the search function to give you some insights
 * Hint2: Don't forget to set the valid bit = 1 in the data item for the rest of functionalities to work
 * Hint3: you will want to check how to use the pwrite function using man pwrite on the terminal
 * 			 ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
 *
 * 	pwrite() writes up to count bytes from the buffer starting  at  buf  to
       the  file  descriptor  fd  at  offset  offset.
 */
int insertItem(int fd, DataItem item)
{
    int count = 0;
    int hashIndex = hashCode(item.key);
    int offset = hashIndex * BUCKETSIZE;
    struct DataItem data;
    ssize_t result;

    for (int numRecord = 0; numRecord < RECORDSPERBUCKET; numRecord++, offset += RECORDSIZE)
    {
        count++;
        result = pread(fd, &data, sizeof(DataItem), offset);
        if (result <= 0) //either an error happened in the pread or it hit an unallocated space
        {                // perror("some error occurred in pread");
            return -1;
        }
        if (data.valid == 0)
        {
            item.valid = 1;
            result = pwrite(fd, &item, sizeof(DataItem), offset);
            if (result <= 0) //writing error
                return -1;
            //written correctly;
            return count;
        }
    }

    // Look for a free slot in the overflow area
    struct OverflowBucket overflowData, overflowItem;
    overflowItem.valid = 1;
    overflowItem.data = item.data;
    overflowItem.key = item.key;
    overflowItem.nextOffset = -1;

    int freeOffset = -1;
    for (int numOverflowBucket = 0; numOverflowBucket < NOVERFLOW_BUCKETS; numOverflowBucket++)
    {
        int auxOffset = MBUCKETS * BUCKETSIZE + numOverflowBucket * OVERFLOW_BUCKETSIZE;
        result = pread(fd, &overflowData, sizeof(OverflowBucket), auxOffset);
        if (result < 0)
        {
            perror("some error occurred in pread");
            return -1;
        }
        else if (overflowData.valid == 0)
        {
            result = pwrite(fd, &overflowItem, sizeof(OverflowBucket), auxOffset);
            if (result <= 0) //writing error
                return -1;
            //written correctly;
            freeOffset = auxOffset;
            break;
        }
    }
    if (freeOffset == -1)
        return -1;

    int overflowLocation;
    result = pread(fd, &overflowLocation, sizeof(int), offset);
    if (result <= 0) //either an error happened in the pread or it hit an unallocated space
    {                // perror("some error occurred in pread");
        return -1;
    }

    while (overflowLocation > 0)
    {
        count++;
        offset = overflowLocation + 3 * sizeof(int);
        result = pread(fd, &overflowData, sizeof(OverflowBucket), overflowLocation);
        if (result <= 0) //either an error happened in the pread or it hit an unallocated space
        {                // perror("some error occurred in pread");
            return -1;
        }
        overflowLocation = overflowData.nextOffset;
    }

    result = pwrite(fd, &freeOffset, sizeof(int), offset);
    if (result <= 0) //writing error
        return -1;
    return count + 1;
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

int searchItem(int fd, struct OverflowBucket *item, int *count, int *parent)
{
    *count = 0;
    *parent = -1;
    int hashIndex = hashCode(item->key);
    int offset = hashIndex * BUCKETSIZE;
    struct DataItem data;
    ssize_t result;

    for (int numRecord = 0; numRecord < RECORDSPERBUCKET; numRecord++, offset += RECORDSIZE)
    {
        (*count)++;
        result = pread(fd, &data, sizeof(DataItem), offset);
        if (result <= 0) //either an error happened in the pread or it hit an unallocated space
        {                // perror("some error occurred in pread");
            return -1;
        }
        if (data.valid == 1 && data.key == item->key)
        {
            item->data = data.data;
            item->nextOffset = -1;
            if (result <= 0) //writing error
                return -1;

            return offset;
        }
    }

    int overflowLocation;
    result = pread(fd, &overflowLocation, sizeof(int), offset);
    if (result <= 0) //either an error happened in the pread or it hit an unallocated space
    {                // perror("some error occurred in pread");
        return -1;
    }

    struct OverflowBucket overflowData;
    while (overflowLocation > 0)
    {
        (*count)++;
        *parent = offset;
        result = pread(fd, &overflowData, sizeof(OverflowBucket), overflowLocation);
        if (result <= 0) //either an error happened in the pread or it hit an unallocated space
        {                // perror("some error occurred in pread");
            return -1;
        }
        if (overflowData.valid == 1 && overflowData.key == item->key)
        {
            item->data = overflowData.data;
            item->nextOffset = overflowData.nextOffset;
            if (result <= 0) //writing error
                return -1;

            return overflowLocation;
        }
        offset = overflowLocation + 3 * sizeof(int);
        overflowLocation = overflowData.nextOffset;
    }
    return -1;
}

/* Functionality: Display all the file contents
 *
 * Input:  fd: filehandler which contains the db
 *
 * Output: no. of non-empty records
 */
int DisplayFile(int fd)
{

    struct DataItem data;
    int ofBucketOffset;
    int offset = 0;
    int count = 0;
    ssize_t result;
    printf("*******Main Buckets*******\n");

    for (int numBucket = 0; numBucket < MBUCKETS; numBucket++)
    {
        offset = numBucket * BUCKETSIZE;
        for (int numRecord = 0; numRecord < RECORDSPERBUCKET; numRecord++, offset += RECORDSIZE)
        {
            result = pread(fd, &data, sizeof(DataItem), offset);
            if (result < 0)
            {
                perror("some error occurred in pread");
                return -1;
            }
            else if (result == 0 || data.valid == 0)
            { //empty space found or end of file
                printf("Bucket: %d, Offset %d:~\n", numBucket, offset);
            }
            else
            {
                printf("Bucket: %d, Offset: %d, Key: %d, Data: %d\n", numBucket, offset, data.key, data.data);
                count++;
            }
        }
        result = pread(fd, &ofBucketOffset, sizeof(int), offset);
        if (result < 0)
        {
            perror("some error occurred in pread");
            return -1;
        }
        else
        {
            printf("Overflow Bucket Offset: %d\n", ofBucketOffset);
        }
    }

    struct OverflowBucket overflowData;
    printf("*******Overflow Buckets*******\n");

    for (int numOverflowBucket = 0; numOverflowBucket < NOVERFLOW_BUCKETS; numOverflowBucket++)
    {
        offset = MBUCKETS * BUCKETSIZE + numOverflowBucket * OVERFLOW_BUCKETSIZE;
        result = pread(fd, &overflowData, sizeof(OverflowBucket), offset);
        if (result < 0)
        {
            perror("some error occurred in pread");
            return -1;
        }
        else if (result == 0 || overflowData.valid == 0)
        { //empty space found or end of file
            printf("Overflow Bucket: %d, Offset %d:~\n", numOverflowBucket, offset);
        }
        else
        {
            printf("Overflow Bucket: %d, Offset: %d, Key: %d, Data: %d, Next: %d\n", numOverflowBucket, offset, overflowData.key, overflowData.data, overflowData.nextOffset);
            count++;
        }
    }
    printf("---------------------------------------\n");
    return count;
}

/* Functionality: Delete item at certain offset
 *
 * Input:  fd: filehandler which contains the db
 *         Offset: place where it should delete
 *
 * Hint: you could only set the valid key and write just and integer instead of the whole record
 */
int deleteOffset(int fd, int Offset, int parent, int child)
{
    int result;
    if (parent != -1)
    {
        result = pwrite(fd, &child, sizeof(int), parent);
        if (result <= 0) //writing error
            return -1;
        struct OverflowBucket dummyItem;
        dummyItem.valid = 0;
        dummyItem.nextOffset = -1;
        result = pwrite(fd, &dummyItem, sizeof(OverflowBucket), Offset);
    }
    else
    {
        struct DataItem dummyItem;
        dummyItem.valid = 0;
        result = pwrite(fd, &dummyItem, sizeof(DataItem), Offset);
    }
    return result;
}

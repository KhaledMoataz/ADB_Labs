/*
 * readfile.h
 *
 *  Created on: Sep 20, 2016
 *      Author: dina
 */

#ifndef READFILE_H_
#define READFILE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MBUCKETS 10                                                                // Number of BUCKETS
#define NOVERFLOW_BUCKETS 10                                                       // No. of extra overflow records for chaining
#define RECORDSPERBUCKET 2                                                         // No. of records inside each Bucket
#define BUCKETSIZE sizeof(Bucket)                                                  // Size of the bucket (in bytes)
#define OVERFLOW_BUCKETSIZE sizeof(OverflowBucket)                                 // Size of overflow bucket (in bytes)
#define RECORDSIZE sizeof(DataItem)                                                // Size of record (in bytes)
#define FILESIZE (BUCKETSIZE * MBUCKETS + NOVERFLOW_BUCKETS * OVERFLOW_BUCKETSIZE) // Size of the file

//Data Record inside the file
struct DataItem
{
   int valid; // means invalid record, 1 = valid record
   int data;
   int key;
};

//Each bucket contains number of records
struct Bucket
{
   struct DataItem dataItem[RECORDSPERBUCKET];
   int overflowBucketOffset;
};

struct OverflowBucket
{
   int valid;
   int data;
   int key;
   int nextOffset;
};

//Check the create File
int createFile(int size, char *);

//check the openAddressing File
int deleteItem(int key);
int insertItem(int fd, DataItem item);
int DisplayFile(int fd);
int deleteOffset(int filehandle, int Offset, int parent, int child);
int searchItem(int filehandle, struct OverflowBucket *item, int *count, int *parent);

#endif /* READFILE_H_ */

#ifndef READFILE_H_
#define READFILE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include<bits/stdc++.h>
using namespace std;

//constants
const int RECORDSPERBUCKET  = 2; 
const int MOD = 32;        //used in hash function
const int LEN = 5;         //log2(MOD)

//Data Record inside the file
struct DataItem {
   int valid;
   int data;     
   int key;
};


//Each bucket contains number of records
struct Bucket {
   int depth;
   int valid;
   DataItem data[RECORDSPERBUCKET];
};


const int BUCKETSIZE = sizeof(Bucket);		//Size of the bucket (in bytes)

//hashing functions 
string To_Binary(int number);
int hashCode(int key,int depth);

//file functions
int createFile(int size,char *);
int extendTable(int fd,int FileSize,string oldName,string name,int modifiedBucket,int newBucketAddress);
void modifyTable(int fd,int index,int depth,int GDepth,int offset);
int Shrink(int td, int fd, int FILESIZE, int & GlobalDepth, string oldName, string &newName);
int ShrinkTable(int fd,int FileSize,int factor,string oldName,string name);

//utility functions
void resetBucket(Bucket & b, int depth);
int AddBucket(int fd,int fileSize,int depth);
int findBucket(int fd, int FILESIZE, int depth);
int countRecords(const Bucket & b);
bool mergeBuckets(int td,int fd,int index,int sib,int base, int diff);
void reDistribute(int fd,int offset1,int offset2);

//printing functions
void DisplayTable(int fd,int FILESIZE);
void DisplayFile(int fd,int FILESIZE);

//main functions
int insertItem(int fd,int BucketAddress,DataItem item);
int searchItem(int fd,int offset,int key);
int deleteItem(int fd,int offset,int key);




#endif /* READFILE_H_ */

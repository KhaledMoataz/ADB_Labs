#include "readfile.h"

int GlobalDepth;              //Table's global depth
int tablehandle,filehandle;   //Handler for the database file
int FILESIZE,TABLESIZE;       //File sizes
string tableName, fileName;   //File names

void Insert(int, int);
int Search(int);
void Delete(int);
void Init();


int main(int argc, char *argv[])
{
   FILESIZE = (1 << GlobalDepth)*sizeof(Bucket);
   TABLESIZE = (1 << GlobalDepth)*sizeof(int);
   tableName = argv[2];
   fileName  = argv[3];
   tablehandle = createFile(TABLESIZE, argv[2]);
   filehandle = createFile(FILESIZE, argv[3]);
   Init();
   
   Insert(2369,1);
   Insert(3760,2);
   Insert(4692,3);
   Insert(4871,4);
   Insert(5659,5);
   Insert(1821,6);
   Insert(1074,7);
   Insert(7115,8);
   Insert(1620,9);
   Insert(2428,10);
   Insert(3943,11);
   Insert(4750,12);
   Insert(6975,13);
   Insert(4981,14);
   Insert(9208,15);
   DisplayTable(tablehandle,TABLESIZE);
   DisplayFile(filehandle,FILESIZE);

   Search(2428);
   Search(1);
   Search(1620);

   Delete(4871);
   DisplayTable(tablehandle,TABLESIZE);
   DisplayFile(filehandle,FILESIZE);

   Insert(4871,4);
   DisplayTable(tablehandle,TABLESIZE);
   DisplayFile(filehandle,FILESIZE);

   Delete(1620);
   DisplayTable(tablehandle,TABLESIZE);
   DisplayFile(filehandle,FILESIZE);

   Insert(1620,9);
   DisplayTable(tablehandle,TABLESIZE);
   DisplayFile(filehandle,FILESIZE);
   
   close(tablehandle);
   close(filehandle);
   return 0;
}

/**
 * @brief Initializes files
 * 
 */
void Init()
{
   int address = 0;
   Bucket bucket;
   bucket.depth = 0;
   for(int i=0;i<RECORDSPERBUCKET;i++) 
      bucket.data[i].valid = 0;
   pwrite(filehandle, &bucket, sizeof(Bucket),0);
   pwrite(tablehandle, &address, sizeof(int), 0);
}

/**
 * @brief Insert a record
 * 
 * @param key 
 * @param data 
 */
void Insert(int key,int data){
   struct DataItem item ;
   item.data = data;
   item.key = key;
   item.valid = 1;

   int index = hashCode(key, GlobalDepth);
   int Bucket_offset;
   pread(tablehandle, &Bucket_offset,sizeof(int),index * sizeof(int));
   
   int LocalDepth = insertItem(filehandle,Bucket_offset,item); 
   if(LocalDepth >= 0)   //no empty places, split
   {
      int address;
      if(LocalDepth == GlobalDepth)  //table split
      {
         GlobalDepth++;
         address = min(FILESIZE, findBucket(filehandle, FILESIZE, GlobalDepth-1));
         if(address == FILESIZE)
            FILESIZE = AddBucket(filehandle,FILESIZE,GlobalDepth-1);
         tablehandle = extendTable(tablehandle,TABLESIZE,tableName,tableName + "0",index,address);
         TABLESIZE *= 2;
         tableName += "0";
      }
      else                    //bucket split
      {
         address = min(FILESIZE, findBucket(filehandle, FILESIZE, LocalDepth));
         if(address == FILESIZE)
            FILESIZE = AddBucket(filehandle,FILESIZE,LocalDepth);
         modifyTable(tablehandle,index,LocalDepth,GlobalDepth,address);
      }
      reDistribute(filehandle,Bucket_offset,address);
      Insert(key,data);
   }
}

/**
 * @brief Search for a key
 * 
 * @param key 
 * @return int 
 */
int Search(int key)
{
   int index = hashCode(key, GlobalDepth);
   int bucket_offset;
   pread(tablehandle, &bucket_offset, sizeof(int), index * sizeof(int));
   return searchItem(filehandle, bucket_offset, key);
}

/**
 * @brief Delete a key
 * 
 * @param key 
 */
void Delete(int key)
{
   int index = hashCode(key, GlobalDepth);
   int bucket_offset;
   pread(tablehandle, &bucket_offset, sizeof(int), index * sizeof(int));

   int result = deleteItem(filehandle, bucket_offset, key);
   if(result < 0)
      return;
   
   index = index >> (GlobalDepth - result);
   int sib = index ^ 1, base = sib;
   
   while(mergeBuckets(tablehandle, filehandle, index << (GlobalDepth - result), sib << (GlobalDepth - result), base, GlobalDepth - result) && result)
      result--, index >>= 1, sib = index ^ 1, base = sib;
   
   tablehandle = Shrink(tablehandle,filehandle,FILESIZE, GlobalDepth, tableName, tableName);
   TABLESIZE = (1 << GlobalDepth) * sizeof(int);
}
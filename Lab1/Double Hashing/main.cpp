//============================================================================
// Name        : hashskeleton.cpp
// Author      :
// Version     :
// Copyright   : Code adapted From https://www.tutorialspoint.com/
// Description : Hashing using open addressing
//============================================================================

#include "readfile.h"
#include <iostream>
#include <fstream>
using namespace std;

void insert(int key, int data);
int deleteItem(int key);
struct DataItem *search(int key);

int filehandle; //handler for the database file

/* DBMS (DataBase Management System) needs to store its data in something non-volatile
 * so it stores its data into files (manteqy :)).

 * Some DBMS or even file-systems constraints the size of that file. 

 * for the efficiency of storing and retrieval, DBMS uses hashing 
 * as hashing make it very easy to store and retrieve, it introduces 
 * another type of problem which is handling conflicts when two items
 * have the same hash index

 * In this exercise we will store our database into a file and experience
 * how to retrieve, store, update and delete the data into this file 

 * This file has a constant capacity and uses external-hashing to store records,
 * however, it suffers from hashing conflicts.
 * 
 * You are required to help us resolve the hashing conflicts 

 * For simplification, consider the database has only one table 
 * which has two columns key and data (both are int)

 * Functions in this file are just wrapper functions, the actual functions are in openAddressing.cpp

*/

int main(int argc, char *argv[])
{
   filehandle = createFile(FILESIZE, argv[2]);

   ifstream myfile(argv[1]);
   while (!myfile.eof())
   {
      int type, key, data;
      myfile >> type;
      switch (type)
      {
      case 1: // insert
         myfile >> key >> data;
         insert(key, data);
         break;
      case 2: // search
         myfile >> key;
         search(key);
         break;
      case 3: // delete
         myfile >> key;
         deleteItem(key);
         break;
      default:
         DisplayFile(filehandle);
      }
   }
   close(filehandle);
   return 0;
}

/* functionality: insert the (key,data) pair into the database table
                  and print the number of comparisons it needed to find
    Input: key, data
    Output: print statement with the no. of comparisons
*/
/* functionality: insert the (key,data) pair into the database table
                  and print the number of comparisons it needed to find
    Input: key, data
    Output: print statement with the no. of comparisons
*/
void insert(int key,int data){
     struct DataItem item ;
     item.data = data;
     item.key = key;
     item.valid = 1;
     int result= insertItem(filehandle,item);  //TODO: implement this function in openAddressing.cpp
     if(result!=-1){
        printf("Insert: No. of searched records:%d\n",abs(result));

     }
     else{
        printf("Insert: Error, Database is full\n");
     }
     
     
}

/* Functionality: search for a data in the table using the key

   Input:  key
   Output: the return data Item
*/
struct DataItem * search(int key)
{
  struct DataItem* item = (struct DataItem *) malloc(sizeof(struct DataItem));
     item->key = key;
     int diff = 0;
     int Offset= searchItem(filehandle,item,&diff); //this function is implemented for you in openAddressing.cpp
     printf("Search: No of records searched is %d\n",diff);
     if(Offset <0)  //If offset is negative then the key doesn't exists in the table
       printf("Item not found\n");
     else
        printf("Item found at Offset: %d,  Data: %d and key: %d\n",Offset,item->data,item->key);
  return item;
}

/* Functionality: delete a record with a certain key

   Input:  key
   Output: return 1 on success and -1 on failure
*/
int deleteItem(int key){
   struct DataItem* item = (struct DataItem *) malloc(sizeof(struct DataItem));
   item->key = key;
   int diff = 0;
   int Offset= searchItem(filehandle,item,&diff);
   printf("Delete: No of records searched is %d\n",diff);
   if(Offset >=0 )
   {
    return deleteOffset(filehandle,Offset);
   }
   printf("Delete: Error, Item not found for Deletion\n");
   return -1;
}

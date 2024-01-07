#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <hashtable.h>

#define SUCCESS 1
#define RESIZED 2
#define FAILED 0

#define INTIIAL_INPUT_SIZE 5 // Size of initial input array

#define MAX_LENGTH 255 // Max input length

/* Gets input from stdin and populating the keys  and values arrays (passed as pointers), and adding the count to *nElem, stringBuffer is a string buffer of MAX_LENGTH which holds the input strings to parse. Returns bool flag indicating success */
bool getInput(char*** keysptr, char*** valuesptr,  char* stringBuffer, int* nElem);

/* Utility to reads a string from stdin */
void readString(char* stringBuffer);

/*Increase nElem and resize the arrays, if nElem is > size*/
int incElem(char*** keysptr, char*** valuesptr, int* nElem, int* size);

size_t oneString = sizeof(char*); //Size of a char pointer, which stores a string address

int main() {
    
   //clear the console :)
   #ifdef _WIN32
   system("cls");
   #else
   system("clear");
   #endif
   
   printf("\n## HASHTABLE DEMO ##\n\n");
   
   printf("Enter key,value. To stop enter '.'\n");
   
   //String buffer for capturing input
   char stringBuffer[MAX_LENGTH];
    
   //Keys and values, initial size INTIIAL_INPUT_SIZE
   char** keys = malloc(oneString * INTIIAL_INPUT_SIZE);
   char** values = malloc(oneString * INTIIAL_INPUT_SIZE);
    
   if (keys == NULL || values == NULL) {
        fprintf(stderr, "Memory alloc failed\n");
        return 1;
    }

   int nElem = 0;
    
   if (!getInput(&keys, &values, stringBuffer, &nElem)) {
       return 1;
    }
    
    HashTable hashtable;
  
    getHashTable(&hashtable,nElem);
    
    if(hashtable == NULL) {
        fprintf(stderr, "HashTable alloc failed\n");
        return 1;
    } 
   
    bool success = rehash(&hashtable, keys, values, nElem);
    
    // Free the keys and values
    for (int i=0; i<nElem; i++) {
        free(keys[i]); free(values[i]);
    }
    free(keys); free(values);
    keys = NULL; values = NULL;
    
    if (!success) {return 1;}

   printHashTable(hashtable);
   
   char stringBuffer2[MAX_LENGTH]; // for receiving value updates
   
   //Interactive loop for CRUD operations
   while(true) {
       printf("\nEnter key to SEARCH, '/' to skip, '.' to quit: \n");
       
       readString(stringBuffer);
       if (!strcmp(".",stringBuffer)) {
           break;
        }
        if (strcmp("/",stringBuffer)) {
           char* value = getValue(hashtable, stringBuffer);
           if (value) {
               printf("\n'%s'=>'%s'\n", stringBuffer, value);
               printf("\nEnter new value to UPDATE for key '%s','/' to skip,  or '.' to quit: \n", stringBuffer);
               readString(stringBuffer2);
               if (!strcmp(".",stringBuffer2)) {
                       break;
                    }
                if (strcmp("/",stringBuffer2)) {
                update(hashtable, stringBuffer, stringBuffer2);
                printHashTable(hashtable);
                }
            } else {
                  printf("\nNo value found for key '%s'\n", stringBuffer);
            }
        }
       
       printf("\nEnter key to DELETE, '/' to skip,  '.' to quit: \n");
       
       readString(stringBuffer);
       if (!strcmp(".",stringBuffer)) {
           break;
        }
        if (strcmp("/",stringBuffer)) {
           del(hashtable, stringBuffer);
           printHashTable(hashtable);
       }
       
       printf("\nEnter key,value to ADD, '/' to skip,  '.' to quit: \n");
       
       readString(stringBuffer);
       if (!strcmp(".",stringBuffer)) {
           break;
        }
       if (strcmp("/",stringBuffer)) {
           char* comma = NULL;
           while(true) {
              comma = strchr(stringBuffer, ',');
              if (comma == NULL || strlen(stringBuffer)<3) {
                  fprintf(stderr, "!Invalid input - format is key,value.\nTry again: \n");
                  readString(stringBuffer);
               } else {
                   break;
               }
           }
           
          *comma = '\0';
          char *key = stringBuffer;
          char *value = &comma[1];
          
          if (key == NULL || value == NULL) {
              fprintf(stderr, "Invalid input\n");
           } else {
               if (checkKey(hashtable, key)) {
                    printf("Duplicate key: %s, updating\n",key);
                    update(hashtable, key, value);
               } else {
                   
                   if(!rehash(&hashtable, &key, &value, 1)) {
                       return 1;
                   }
                   
               }
      
               printHashTable(hashtable);
           }
       }
    }
    
    freeHashTable(&hashtable);
}

bool getInput(char*** keysptr, char*** valuesptr, char* stringBuffer, int* nElem) {

   char** keys = *keysptr;
   char** values = *valuesptr;
    
   int size  = INTIIAL_INPUT_SIZE;
    
   while(true) {
       
       readString(stringBuffer);
       
       if (!strcmp(".",stringBuffer)) {
           break;
        }
       char* comma = strchr(stringBuffer, ',');
       if (comma == NULL || strlen(stringBuffer)<3) {
           fprintf(stderr, "!Invalid input - format is key,value.\nTry again: \n");
           continue;
        }
       *comma = '\0';
       char* key = stringBuffer;
       char* value = &comma[1];
      
       if (key == NULL || value == NULL) {
           fprintf(stderr, "Invalid input\n");
           return false;
        }
        
        for (int i=0; i<(*nElem); i++) {
           if (!strcmp(key,keys[i])) {
               fprintf(stderr, "!Duplicate key %s, ignoring\n", key);
               goto continueOuter;
            }   
        }
       
       //use strdup to get copies, these are allocated on the heap and need to be freed.
       keys[*nElem] = strdup(key);
       values[*nElem] = strdup(value);
       if (keys[*nElem] == NULL || values[*nElem] == NULL) {
           fprintf(stderr, "strdup failed(2)\n");
           return false;
        }
       
       int incStatus = incElem(keysptr,valuesptr, nElem, &size);
       if (incStatus){
           if (incStatus == RESIZED) { //Reassign to new address otherwise will get SEGFAULT
               keys = *keysptr;
               values = *valuesptr;
           }
       } else {
           return false;
       }
       continueOuter:;
   }
   printf("\nN = %d\n",*nElem);
   
   return true;
}

int incElem(char*** keysptr, char*** valuesptr, int* nElem, int* size) {
    
    (*nElem)++;
    //resize arrays if needed
    if ((*nElem) >= (*size)) {
        (*size) = (*nElem)+5;
        printf("<Resizing input arrays to %d>\n\n", *size);
        *keysptr = realloc(*keysptr, oneString * (*size));
        *valuesptr = realloc(*valuesptr, oneString * (*size));
        if (*keysptr == NULL || *valuesptr == NULL) {
            fprintf(stderr, "Input array realloc failed\n");
            return FAILED;
         }
        return RESIZED;
    }
    return SUCCESS;
}

void readString(char* stringBuffer) {
    fgets(stringBuffer, MAX_LENGTH, stdin);
    //Cut trailing "\n"
    stringBuffer[strcspn(stringBuffer, "\n")] = 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#define SUCCESS 1
#define RESIZED 2
#define FAILED 0

#define MAX_LENGTH 255 // Max input length

/* Represents a single key=>value entry in the hashtable
 * next: index of the next entry in the collision chain, if any.
 * If none, next is set to -1*/
typedef struct entry{
  char* key;
  int keyIndex;
  char* value;
  int   next;
} Entry;

typedef Entry* HashTable;

//djb2 hash
int hash(char* str);

void readString(char*);
int getInput(char*** , char*** , bool**, int**, char*);
/*Increase elements and resize backing arrays if needed*/
int incEl(char*** , char***, bool**,int**);
void rehash(HashTable* , char** , char** , int*, bool**, int);

void printHashTable(HashTable, int*);

//CRUD operations
bool checkKey(HashTable, char*);
char* getValue(char* key, HashTable hashTable);
void update(HashTable, char**, char**, char*, char*);
void delete(HashTable, char*, char** , char**, int*);

/*string format length calculation utility*/
int fl(const char*, ...);

/*When sizing the hashtable, prime numbers are preferred
 *for minimal collisions when hashing*/
int nextPrime(int n);

/*globals*/
int nElem=0;      //N elements in backing array
int storedElem=0; //N elements stored in hash table
int tableSize;   //Current hashtable size, always set to a prime number
int size = 5;     //Initial size for backing array

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
    
   //Backing arrays
   char** keys = malloc(oneString * size);
   char** values = malloc(oneString * size);
   int* hashes = malloc(sizeof(int) * size);
    
   //Array of flags indicating stored status of backing array entries
   bool* stored = malloc(sizeof(bool) * size);
    
    
   if (keys == NULL || values == NULL) {
        fprintf(stderr, "Memory alloc failed\n");
        return 1;
    }

   if (getInput(&keys, &values, &stored, &hashes, stringBuffer)) {
       return 1;
    }
    
   //Set tableSize, ensuring load factor of <75%
   tableSize = nextPrime((nElem*4)/3);
  
   HashTable hashtable = malloc(tableSize * sizeof(Entry));
   
   rehash(&hashtable, keys, values, hashes, &stored, 0);

   printHashTable(hashtable, hashes);
   
   char stringBuffer2[MAX_LENGTH]; // for receiving value updates
   
   //Interactive loop for CRUD operations
   while(true) {
       printf("\nEnter key to SEARCH, '/' to skip, '.' to quit: \n");
       
       readString(stringBuffer);
       if (!strcmp(".",stringBuffer)) {
           break;
        }
        if (strcmp("/",stringBuffer)) {
           char* value = getValue(stringBuffer, hashtable);
           if (value) {
               printf("\n'%s'=>'%s'\n", stringBuffer, value);
               printf("\nEnter new value to UPDATE for key '%s','/' to skip,  or '.' to quit: \n", stringBuffer);
               readString(stringBuffer2);
               if (!strcmp(".",stringBuffer2)) {
                       break;
                    }
                if (strcmp("/",stringBuffer2)) {
                update(hashtable, keys, values, stringBuffer, strdup(stringBuffer2));
                printHashTable(hashtable, hashes);
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
           delete(hashtable, stringBuffer, keys, values, hashes);
           printHashTable(hashtable, hashes);
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
                    update(hashtable, keys, values, key, strdup(value));
               } else {
                   int start = nElem;
                    
                   keys[nElem] = strdup(key);
                   values[nElem] = strdup(value);
                   
                   if (keys[nElem] == NULL || values[nElem] == NULL) {
                      fprintf(stderr, "Memory alloc failed(2)\n");
                      return 1;
                   }
                   
                   if (incEl(&keys,&values,&stored,&hashes) == FAILED) {
                       return 1;
                   }
                   
                   if(storedElem > (tableSize * 3)/4) {
                      tableSize = nextPrime((storedElem*4)/3);
                      start = 0;
                     }
                   
                   if (!start) {
                       printf("\nResizing - Full Rehash trigerred!\n");
                       hashtable = realloc(hashtable, tableSize * sizeof(Entry));
                   }
                   
                   rehash(&hashtable, keys, values, hashes, &stored, start);
               }
      
               printHashTable(hashtable, hashes);
           }
       }
    }
    
    
    //Free all allocated memories
    for (int i=0; i<nElem; i++) {
        free(keys[i]); free(values[i]);
    }
    free(keys);free(values);free(stored);free(hashes);free(hashtable);
}

int getInput(char*** keysptr, char*** valuesptr, bool **storedptr, int** hashesptr, char* stringBuffer) {

   char** keys = *keysptr;
   char** values = *valuesptr;
    
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
           return 1;
        }
        
        for (int i=0; i<nElem; i++) {
           if (!strcmp(key,keys[i])) {
               fprintf(stderr, "!Duplicate key %s, ignoring\n", key);
               goto continueOuter;
            }   
        }
       
       
       //use strdup to get copies, these are allocated on the heap and need to be freed.
       keys[nElem] = strdup(key);
       values[nElem] = strdup(value);
       if (keys[nElem] == NULL || values[nElem] == NULL) {
           fprintf(stderr, "Memory alloc failed(2)\n");
           return 1;
        }
       
       int incStatus = incEl(keysptr,valuesptr,storedptr,hashesptr);
       if (incStatus){
           if (incStatus == RESIZED) { //Reassign to new address otherwise will get SEGFAULT
               keys = *keysptr;
               values = *valuesptr;
           }
       } else {
           return 1;
       }
       
       
      
       
       continueOuter:;
       
   }
   printf("\nN = %d\n",nElem);
   
   return 0;
}

int incEl(char*** keysptr, char*** valuesptr, bool** storedptr, int** hashesptr) {
    nElem++;
    storedElem++;
    
    //resize arrays if needed
    if (nElem>=size) {
        size = nElem+5;
        printf("<Resizing backing arrays to %d>\n\n", size);
        *keysptr = realloc(*keysptr, oneString * size);
        *valuesptr = realloc(*valuesptr, oneString * size);
        *storedptr = realloc(*storedptr, sizeof(bool) * size);
        *hashesptr = realloc(*hashesptr, sizeof(int) * size);
        if (*keysptr == NULL || *valuesptr == NULL || *storedptr == NULL || *hashesptr == NULL) {
            fprintf(stderr, "Memory alloc failed(3)\n");
            return FAILED;
         }
        return RESIZED;
    }
    return SUCCESS;
}
  
void rehash(HashTable* hashtableptr, char** keys, char** values, int* hashes, bool** storedptr, int start) {

    printf("\nBuilding Hash Table:\n");
    HashTable hashtable = *hashtableptr;

    bool* stored = *storedptr;
 
    if (start==0) {
        for (int i=start; i<tableSize; i++) {
            hashtable[i].key = NULL;
            hashtable[i].value = NULL;
            hashtable[i].next = -1;
            if (i<nElem) {
                stored[i] = false;
                hashes[i] = hash(keys[i]);
            }
        }
    } else {
        for (int i=start;i<nElem; i++) {
            stored[i] = false;
            hashes[i] = hash(keys[i]);
            printf("Stored #[%s] = %d to hashes\n", keys[i], hashes[i]);
        }
    }
    
   printf("\nStep 1 - Initial Hashes:\n");
   bool collisions = false;
   for (int i=start;i<nElem; i++) {
       if (keys[i] == NULL) continue;
	    int khash = hashes[i];
	    printf("#[%s] = %d\n", keys[i],khash);
	    if (hashtable[khash].key == NULL ) {
	    	hashtable[khash].key = keys[i];
            hashtable[khash].keyIndex = i;
	    	hashtable[khash].value = values[i];
	    	hashtable[khash].next = -1;
	    	stored[i] = true;
	    	printf("Stored %s=>%s at %d\n", keys[i], values[i], khash);
	    } else {
	       collisions = true;
	       printf ("Hash collision!\n")  ;
	    }
   }
   if (collisions) {
       printf("\nStep 2 - Handling collisions by open addressing:\n");
       for (int i=start;i<nElem; i++) {
           if (stored[i] || keys[i] == NULL) continue;
       	   printf("'%s':\n",keys[i]); 
       	   for (int j=0;j<tableSize; j++) {
    	  	if (hashtable[j].key == NULL) {
    	  	    printf("Found empty slot: %d\n", j); 
    	  	  	hashtable[j].key = keys[i];
                hashtable[j].keyIndex = i;
        		hashtable[j].value = values[i];
        		hashtable[j].next = -1;
                int next =  hashes[i];
                printf("%d->%s", next, hashtable[next].key );
        		while( hashtable[next].next != -1) { next = hashtable[next].next;  printf("->%s", hashtable[next].key ); }
        		
        		hashtable[next].next = j;
                printf("->%s=>%s\n",keys[i], values[i]);
        		break;
    	  	}
    	  }
        }
    } else {
        printf("\nNo collisions to handle!\n");
    }
}

bool checkKey(HashTable hashtable, char* key) {
    int next =  hash(key);
    while( next != -1) {
        if ( hashtable[next].key != NULL ) {
            if (!strcmp(key, hashtable[next].key)) {
                return true;
            }
        }
        next = hashtable[next].next; 
    }
     return false;
}

char* getValue(char* key, HashTable hashtable) {
    int next =  hash(key);
    printf("\n#%d", next);
    while( next != -1) {
        printf("->%s", ( hashtable[next].key == NULL ? "ⓧ" : hashtable[next].key ) );
        if ( hashtable[next].key == NULL ) break;
        if (!strcmp(key, hashtable[next].key)) {
            printf("\n");
            return hashtable[next].value;
        }
        
        next = hashtable[next].next;
    }
    return NULL;
}

void update(HashTable hashtable, char** keys, char** values, char* key, char* value) {
    int next =  hash(key);
    printf("\n%d", next);
    while( next != -1) {
        printf("->%s", ( hashtable[next].key == NULL ? "ⓧ" : hashtable[next].key ) );
        if ( hashtable[next].key == NULL ) break;
        if (!strcmp(key, hashtable[next].key)) {
            int i = hashtable[next].keyIndex;
            free(values[i]);
            values[i] = value;
            hashtable[next].value = values[i];
            return;
        }
        
        next = hashtable[next].next; 
    }
     printf("\nKey: '%s' not found\n", key);

}

void delete(HashTable hashtable, char* key, char** keys, char** values, int* hashes) {
    int currentIndex =  hash(key);
    printf("\n%d", currentIndex);
    int prevIndex = -1;
    while( currentIndex != -1) {
        printf("->%s", ( hashtable[currentIndex].key == NULL ? "ⓧ" : hashtable[currentIndex].key ) );
        if ( hashtable[currentIndex].key == NULL ) break;
        if (!strcmp(key, hashtable[currentIndex].key)) {
            printf("\nDeleting key '%s'\n", key);
            int i = hashtable[currentIndex].keyIndex;
            if (prevIndex!= -1) {
                hashtable[prevIndex].next = hashtable[currentIndex].next;
            } else {
                int nextIndex = hashtable[currentIndex].next;
                if (nextIndex != -1) {
                    
                    // Don't chain through non matching hashes which may happen
                    // after multiple deletions and additions
                    
                    while(hashes[hashtable[nextIndex].keyIndex] != hashes[hashtable[currentIndex].keyIndex] ) {
                        nextIndex = hashtable[nextIndex].next;
                        if (nextIndex == -1) break;
                    }
                    if (nextIndex != -1) {
                        hashtable[currentIndex].key = hashtable[nextIndex].key;
                        hashtable[currentIndex].keyIndex = hashtable[nextIndex].keyIndex;
                        hashtable[currentIndex].value = hashtable[nextIndex].value;
                        hashtable[currentIndex].next = hashtable[nextIndex].next;
                        currentIndex = nextIndex;
                    }
                }
            }
            hashtable[currentIndex].keyIndex = -1;
            hashtable[currentIndex].key = NULL;
            hashtable[currentIndex].value = NULL;
            hashtable[currentIndex].next = -1;
            if (i!=-1) {
                free(keys[i]);
                free(values[i]);
                keys[i] = NULL;
                values[i]= NULL;
                hashes[i] = -1;
            }
            
            storedElem--;
            return;
        }
        
        prevIndex = currentIndex;
        currentIndex = hashtable[currentIndex].next;
    }
     printf("\nKey: '%s' not found\n", key);
}


void printHashTable(HashTable hashtable, int* hashes) {
    printf("\nCurrent Table:\n");
    for (int i=0;i<tableSize;i++) {
	   Entry entry = hashtable[i];
       if (entry.key!=NULL) {
           bool root = (i == hashes[entry.keyIndex]);
           bool hasNext = entry.next != -1;
           const char* format = " ->%s[%d]";
           char next[hasNext ? fl(format,hashtable[entry.next].key, entry.next) : 1];
           if (hasNext) {
               sprintf(next,format,hashtable[entry.next].key, entry.next);
           } else {
               strcpy(next,"");
           }
           printf("%d]%s%s=>%s%s\n",i, (root?"":">"), entry.key, entry.value, next);
           
        }
	   else printf("%d]\n",i);
   }
}

int nextPrime(int n) {
    for(int i=n+(1-(n%2))+2;;i+=2) {
        int sqi = (int) sqrt(i);
        for(int j=3;j<=sqi;j+=2) {
            if(i%j==0) goto continueOuter;
        }
        return i;
        continueOuter:;
    }
}

//format length calculation using vsnprintf
int fl(const char* format, ...) {
   int n;
   va_list args;
   va_start(args, format);
   n = vsnprintf(NULL,0,format,args);
   va_end(args);
   return n + 1;
}

void readString(char* stringBuffer) {
    fgets(stringBuffer, MAX_LENGTH, stdin);
    //Cut trailing "\n"
    stringBuffer[strcspn(stringBuffer, "\n")] = 0;
}

//A simpler hash function
int hash0(char* str, int mod) {
    if (mod == 0) return -1;
    int l = strlen(str);
    int ds = 0;
    for (int i=0;i<l;i++) {
            ds += str[i];
    }

    return (ds+l) % mod;
}

//djb2 hash
int hash(char* str) {
        long hash = 5381;
        int c;
        while ((c = (*str++)))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return (int) (hash%tableSize);
}

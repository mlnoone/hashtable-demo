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

//djb2 hash
static int hash(char* str);

/*string format length calculation utility*/
static int fl(const char*, ...);

/*When sizing the hashtable, prime numbers are preferred
 *for minimal collisions when hashing*/
static int nextPrime(int n);

/*Increase element count by newCount and resize arrays if needed*/
static int incNElem(int newCount);

/*globals*/
static int tableSize;   //Current hashtable capacity, always set to a prime number
static int storedElem;  //N elements in stored in hash table

static int nElem=0;      //N elements in keys and values backing arrray
static int size = 5;     //Initial size for keys and values backing arrrays

static size_t oneString = sizeof(char*); //Size of a char pointer, which stores a string address

//Keys and values backing arrays
static char** keys;
static char** values;


void getHashTable(HashTable* htp, int n) {
    
    tableSize = nextPrime((n*4)/3);
    
    size = n + size;
    
    keys = malloc(oneString * size);
    values = malloc(oneString * size);
    
    if (keys == NULL || values == NULL) {
        fprintf(stderr,"Backing arrays malloc failed\n");
        return;
    }
    
    *htp = malloc(tableSize * sizeof(Entry));

}


static bool reSizeHashTableIfNeeded(HashTable* htp) {
    bool resized = false;
    if(storedElem > (tableSize * 3)/4) {
       tableSize = nextPrime((storedElem*4)/3);
        resized = true;
      }
    
    if (resized) {
        printf("\nResizing - Full Rehash trigerred!\n");
        *htp = realloc(*htp, tableSize * sizeof(Entry));
    }
    return resized;
}

static int incNElem(int newCount) {
    nElem+=newCount;
    
    //resize arrays if needed
    if (nElem>=size) {
        size += newCount;
        printf("<Resizing backing arrays to %d>\n\n", size);
        keys = realloc(keys, oneString * size);
        values = realloc(values, oneString * size);
        if (keys == NULL || values == NULL) {
            fprintf(stderr, "Backing array realloc failed\n");
            return FAILED;
         }
        return RESIZED;
    }
    return SUCCESS;
}
  
bool add(HashTable* htp, char** newKeys, char** newValues, int newCount) {
    
    int startElem = nElem;
    if (incNElem(newCount) == FAILED) {
        return false;
    }
    for (int i=0; i < newCount;i++) {
        int g = i+startElem;
        //strdup is used to get local copies of the keys and values, these must be freed
        keys[g] = strdup(newKeys[i]);
        values[g] = strdup(newValues[i]);
        if (keys[g] == NULL || values[g] == NULL) {
            fprintf(stderr,"strdup in HashTable failed\n");
            return false;
        }
    }
    storedElem += newCount;
    
    bool resized = reSizeHashTableIfNeeded(htp);
    
    if (resized) {
        if(*htp == NULL) {
            fprintf(stderr,"HashTable realloc failed\n");
            return false;
        }
        startElem = 0;
    }
    
    printf("\nBuilding Hash Table:\n");
    HashTable hashtable = *htp;

    bool stored[resized ?  nElem : newCount];
    int hashes[resized ? nElem : newCount];
    
    if (startElem==0) {
        for (int i=0; i<tableSize; i++) {
            hashtable[i].key = NULL;
            hashtable[i].value = NULL;
            hashtable[i].next = -1;
            if (i<nElem) {
                stored[i] = false;
                hashes[i] = hash(keys[i]);
            }
        }
    } else {
        for (int i=startElem;i<nElem; i++) {
            int i_s = i - startElem;
            stored[i_s] = false;
            hashes[i_s] = hash(keys[i]);
        }
    }
    
   printf("\nStep 1 - Initial Hashes:\n");
   bool collisions = false;
   for (int i=startElem;i<nElem; i++) {
       int i_s = i - startElem;
       if (keys[i] == NULL) continue;
        int khash = hashes[i_s];
        printf("#[%s] = %d\n", keys[i],khash);
        if (hashtable[khash].key == NULL ) {
            hashtable[khash].key = keys[i];
            hashtable[khash].hash = khash;
            hashtable[khash].keyIndex = i;
            hashtable[khash].value = values[i];
            hashtable[khash].next = -1;
            stored[i_s] = true;
            printf("Stored %s=>%s at %d\n", keys[i], values[i], khash);
        } else {
           collisions = true;
           printf ("Hash collision!\n")  ;
        }
   }
   if (collisions) {
       printf("\nStep 2 - Handling collisions by open addressing:\n");
       for (int i=startElem;i<nElem; i++) {
           int i_s = i - startElem;
           if (stored[i_s] || keys[i] == NULL) continue;
              printf("'%s':\n",keys[i]);
              for (int j=0;j<tableSize; j++) {
              if (hashtable[j].key == NULL) {
                printf("Found empty slot: %d\n", j);
                hashtable[j].key = keys[i];
                hashtable[j].keyIndex = i;
                hashtable[j].hash = hashes[i_s];
                hashtable[j].value = values[i];
                hashtable[j].next = -1;
                int currentIndex =  hashes[i_s];
                printf("%d->%s", currentIndex, hashtable[currentIndex].key );
                // Go to the end of the chain, where next will be -1
                while( hashtable[currentIndex].next != -1) { currentIndex = hashtable[currentIndex].next;  printf("->%s", hashtable[currentIndex].key ); }
                // Mark j as the next for currentIndex in the collision chain
                hashtable[currentIndex].next = j;
                printf("->%s=>%s\n",keys[i], values[i]);
                break;
              }
          }
        }
    } else {
        printf("\nNo collisions to handle!\n");
    }
    return true;
}

bool checkKey(HashTable hashtable, char* key) {
    int currentIndex =  hash(key);
    while( currentIndex != -1) {
        if ( hashtable[currentIndex].key != NULL ) {
            if (!strcmp(key, hashtable[currentIndex].key)) {
                return true;
            }
        }
        currentIndex = hashtable[currentIndex].next;
    }
     return false;
}

char* getValue(HashTable hashtable, char* key) {
    int currentIndex =  hash(key);
    printf("\n#%d", currentIndex);
    while( currentIndex != -1) {
        if ( hashtable[currentIndex].key != NULL ) {
            printf("->%s", hashtable[currentIndex].key );
            if (!strcmp(key, hashtable[currentIndex].key)) {
                printf("\n");
                return hashtable[currentIndex].value;
            }
        }
        currentIndex = hashtable[currentIndex].next;
    }
    printf("->ⓧ");
    return NULL;
}

void update(HashTable hashtable, char* key, char* value) {
    int currentIndex =  hash(key);
    printf("\n%d", currentIndex);
    while( currentIndex != -1) {
        if ( hashtable[currentIndex].key != NULL ) {
            printf("->%s", hashtable[currentIndex].key );
            if (!strcmp(key, hashtable[currentIndex].key)) {
                int i = hashtable[currentIndex].keyIndex;
                printf("=>%s\n",value);
                //Free current value and insert local copy of new value
                free(values[i]);
                values[i] = strdup(value);
                
                hashtable[currentIndex].value = values[i];
                return;
            }
        }
        
        currentIndex = hashtable[currentIndex].next;
        
    }
    printf("->ⓧ");
    printf("\nKey: '%s' not found\n", key);

}

void del(HashTable hashtable, char* key) {
    int currentIndex =  hash(key);
    printf("\n%d", currentIndex);
    int prevIndex = -1;
    while( currentIndex != -1) {
        if ( hashtable[currentIndex].key != NULL ) {
            printf("->%s", hashtable[currentIndex].key );
            if (!strcmp(key, hashtable[currentIndex].key)) {
                printf("\nDeleting key '%s'\n", key);
                int keyIndex = hashtable[currentIndex].keyIndex;
                int nextIndex = hashtable[currentIndex].next;
                // If next is not -1, and not a root (this can happen after multiple additions and deletions)
                // then shift it to the currentIndex
                bool shiftNext = !(nextIndex == -1 || nextIndex == hashtable[nextIndex].hash);
                if (shiftNext) {
                    hashtable[currentIndex].key = hashtable[nextIndex].key;
                    hashtable[currentIndex].hash = hashtable[nextIndex].hash;
                    hashtable[currentIndex].keyIndex = hashtable[nextIndex].keyIndex;
                    hashtable[currentIndex].value = hashtable[nextIndex].value;
                    hashtable[currentIndex].next = hashtable[nextIndex].next;
                    currentIndex = nextIndex;
                }
                if (!(prevIndex == -1 || shiftNext)) {
                    // if next is not shifted, and previous is not empty,
                    // link next to previous one, as current is going to deleted
                    // otherwise this step is not needed
                    hashtable[prevIndex].next = hashtable[currentIndex].next;
                }
                hashtable[currentIndex].keyIndex = -1;
                hashtable[currentIndex].hash = -1;
                hashtable[currentIndex].key = NULL;
                hashtable[currentIndex].value = NULL;
                hashtable[currentIndex].next = -1;
                
                //Free up memory of the unused keys and values
                free(keys[keyIndex]);
                free(values[keyIndex]);
                keys[keyIndex] = NULL;
                values[keyIndex]= NULL;
                
                //Decrement stored elem
                storedElem --;
                return;
            }
        }
        prevIndex = currentIndex;
        currentIndex = hashtable[currentIndex].next;
    
    }
    printf("->ⓧ");
    printf("\nKey: '%s' not found\n", key);
}


void printHashTable(HashTable hashtable) {
    printf("\nCurrent Table:\n");
    for (int i=0;i<tableSize;i++) {
       Entry entry = hashtable[i];
       if (entry.key!=NULL) {
           bool root = (i == entry.hash);
           bool hasNext = entry.next != -1;
           const char* format = " ->%s[%d]";
           int nextStringLength = hasNext ? fl(format,hashtable[entry.next].key, entry.next) : 1;
           char nextString[nextStringLength];
           if (hasNext) {
               snprintf(nextString,nextStringLength,format,hashtable[entry.next].key, entry.next);
           } else {
               strcpy(nextString,"");
           }
           printf("%d]%s%s=>%s%s\n",i, (root?"":">"), entry.key, entry.value, nextString);
           
        }
       else printf("%d]\n",i);
   }
}

void freeHashTable(HashTable* htp) {
    for (int i=0; i<nElem; i++) {
        free(keys[i]); free(values[i]);
    }
    free(keys);
    free(values);
    free(*htp);
    keys = NULL;
    values = NULL;
    *htp = NULL;
}

static int nextPrime(int n) {
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
static int fl(const char* format, ...) {
   int n;
   va_list args;
   va_start(args, format);
   n = vsnprintf(NULL,0,format,args);
   va_end(args);
   return n + 1;
}

/*
//A simpler hash function
static int hash0(char* str, int mod) {
    if (mod == 0) return -1;
    int l = strlen(str);
    int ds = 0;
    for (int i=0;i<l;i++) {
            ds += str[i];
    }

    return (ds+l) % mod;
}*/

//djb2 hash
static int hash(char* str) {
        long hash = 5381;
        int c;
        while ((c = (*str++)))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return (int) (hash%tableSize);
}

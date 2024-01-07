/* Represents a single key=>value entry in the hashtable
 * next: index of the next entry in the collision chain, if any.
 * If none, next is set to -1*/
typedef struct entry{
  char* key;
  int hash;  //stores the key hash
  int keyIndex; //stores the index of key in backing array (which is same as index of value)
  char* value;
  int   next;
} Entry;

//A HashTable is an array of Entries!
typedef Entry* HashTable;

/* Allocates a hash table to the passed pointer with intial capacity for holding n items
 This must be freed by the calling freeHashtable function when done*/
void getHashTable(HashTable* htp, int n);

void printHashTable(HashTable hashtable);

//CRUD operations
/* Add newCount entries from newKeys and newValues arrays, local copies of the each string are stored, the hashtable will be rehashed if needed*/
bool add(HashTable* htp, char** newKeys, char** newValues, int newCount);
bool checkKey(HashTable hashtable, char* key);
char* getValue(HashTable hashtable, char* key);
//Note: Stores a local copy of value, after clearing the previous one
void update(HashTable hashtable, char* key, char* value);
void del(HashTable hashtable, char* key);

//Free all allocated memories
void freeHashTable(HashTable* htp);

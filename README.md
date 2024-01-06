## hashtable-demo

_An interactive hash table demo in c_
![Screenshot](https://sasyabook.com/images/htblss.png)
Ah... the amazing hash table, the backbone of much of modern software. Associative arrays, dictionaries, most PHP data structures, are all hash tables behind the scene.

Hash tables combine flexible storage with quick addressing, and handle all of C-R-U-D efficiently and elegantly.

This started as a self learning project to explore the workings of the hashtable.

You can  try it out quickly by pasting the code inside 'hashtable.c' at the Online C Compiler, https://www.onlinegdb.com/online_c_compiler

or it can be compiled and run on Linux, Mac or Windows: `gcc -o hashtable hashtable.c` [with ` -lm` in linux]

The code is commented to highlight some important paradigms of hash tables.

Note: Collisions are handled by open adressing / linear probing.

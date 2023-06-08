/*
 * HASH_table.c
 *
 * Copyright (C) 2023 Mera Emmanuel <emmanuel.mera@live.fr>.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef HASH_table_h
#define HASH_table_h

#include <stdlib.h>

#define HASH_MIN_TABLE_LEN  128
#define HASH_DEFAULT_LEN    16384

#define RANDOM_SEED         -1

typedef struct {
    void        *next;
    const char  *name;
    void        *element;
    int         len;
} hashList;

typedef struct {
    hashList        **table;
    size_t          len;
    size_t          count;
    
    hashList        **oldtable;
    size_t          oldlen;
    size_t          oldcount;
    size_t          index;
    
    unsigned int    (*hash) (const char *, size_t, int);
    int             seed;
} KER_hashTable;

/**
 * Create a hash table of initial length the lowest power of two above 'len'
 */
extern KER_hashTable *KER_hash_create_table(size_t len);

/**
 * Delete the hash table.
 */
extern void KER_hash_delete_table(KER_hashTable *tbl);

/**
 * Add an element to the hash table. The given key will not be copied, only a pointer to it will be stored.
 */
extern void KER_hash_add(KER_hashTable *tbl, const char *key, int len, void *element);

/**
 * Find an element in the hash table. If no elements were found, a NULL pointer is returned.
 */
extern void *KER_hash_find(KER_hashTable *tbl, const char *key, int len);

/**
 * Set the seed for the hash function. The seed is set to 0 by default.
 */
extern void KER_hash_set_seed(KER_hashTable *tbl, int seed);

/**
 * Change the hash function (by default, the xxhash32 algorithm is used). The prototype of the funciton must be : unsigned int foo(const char *key, size_t len, int seed);
 */
extern void KER_hash_set_hash_func(KER_hashTable *tbl, unsigned int (*hash) (const char *, size_t, int));

#endif /* HASH_table_h */

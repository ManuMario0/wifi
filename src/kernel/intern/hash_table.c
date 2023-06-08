/*
 * HASH_table.c
 *
 * Copyright (C) 2023 Mera Emmanuel <emmanuel.mera@live.fr>.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include <string.h> // strlen

#include "xxhash.h"

#include "KER_table.h"

#include "MEM_alloc.h"

/* ------------------------ */
/* local functions          */
/* ------------------------ */

static void add_item(KER_hashTable *tbl, const char *name, int len, void *element);
static void update_table(KER_hashTable *tbl);

/* ------------------------ */
/* implementation           */
/* ------------------------ */

KER_hashTable *KER_hash_create_table(size_t len) {
    KER_hashTable *tbl = MEM_calloc(sizeof(KER_hashTable), __func__);
    if (tbl == NULL) {
        return NULL;
    }
    
    /* set a minimum value for the size */
    if (len < HASH_MIN_TABLE_LEN) {
        len = HASH_MIN_TABLE_LEN;
    }
    
    /* the len must be a power of two for performance */
    if (((len - 1) & len) != 0) {
        int nlen = 1;
        while (len != 0) {
            len >>= 1;
            nlen <<= 1;
        }
        len = nlen;
    }
    
    tbl->table = (hashList **)MEM_calloc_array(len, sizeof(hashList*), __func__);
    if (tbl->table == NULL) {
        MEM_free(tbl);
        return NULL;
    }
    tbl->len = len;
    tbl->count = 0;
    
    tbl->oldtable = NULL;
    tbl->oldlen = 0;
    tbl->oldcount = 0;
    tbl->index = 0;
    
    tbl->hash = (unsigned int (*)(const char *, size_t, int))&XXH3_64bits;
    tbl->seed = 0;
    
    return tbl;
}

void KER_hash_set_seed(KER_hashTable *tbl, int seed) {
    if (tbl == NULL) {
        return;
    }
    if (tbl->count != 0 || tbl->oldcount != 0) {
        return;
    }
    
    if (seed == RANDOM_SEED) {
        tbl->seed = 1; // will set randomness later
        return;
    }
    
    if (seed < 0) {
        return;
    }
    tbl->seed = seed;
}

void KER_hash_add(KER_hashTable *tbl, const char *name, int len, void *element) {
    if (tbl == NULL) return;
    char *id = MEM_malloc(len, __func__);
    memcpy(id, name, len);
    add_item(tbl, id, len, element);
    update_table(tbl);
}

void *KER_hash_find(KER_hashTable *tbl, const char *name, int len) {
    if (tbl == NULL) {
        return NULL;
    }
    unsigned int hash = (*tbl->hash)(name, len, tbl->seed);
    unsigned int index = hash % tbl->len;
    
    hashList *lst = tbl->table[index];
    while (lst != NULL && ((lst->len != len) || memcmp(lst->name, name, len) != 0)) {
        lst = lst->next;
    }
    
    if (lst) {
        return lst->element;
    }
    
    if (tbl->oldtable) {
        index = hash % tbl->oldlen;
        
        lst = tbl->oldtable[index];
        while (lst != NULL && ((lst->len != len) || memcmp(lst->name, name, len) != 0)) {
            lst = lst->next;
        }
        
        if (lst) {
            return lst->element;
        }
    }
    return NULL;
}

void **HASH_find_all(KER_hashTable *tbl, const char *name) {
    return NULL;
}

void KER_hash_set_hash_func(KER_hashTable *tbl, unsigned int (*hash) (const char *, size_t, int)) {
    if (tbl->count != 0 || tbl->oldcount != 0) return;
    tbl->hash = (unsigned int (*)(const char *, size_t, int))&hash;
}

void KER_hash_delete_table(KER_hashTable *tbl) {
    if (tbl->oldtable) {
        for (int i=0; i<tbl->oldlen; i++) {
            hashList *lst = tbl->oldtable[i];
            if (lst) {
                while (lst) {
                    hashList *flst = lst;
                    lst = lst->next;
                    MEM_free(flst);
                }
            }
        }
        MEM_free(tbl->oldtable);
    }
    for (int i=0; i<tbl->len; i++) {
        hashList *lst = tbl->table[i];
        if (lst) {
            while (lst) {
                hashList *flst = lst;
                lst = lst->next;
                MEM_free(flst);
            }
        }
    }
    
    MEM_free(tbl->table);
    MEM_free(tbl);
}

/* ------------------------ */
/* local functions          */
/* ------------------------ */

static void add_item(KER_hashTable *tbl, const char *name, int len, void *element)
{
    unsigned int hash = (*tbl->hash)(name, len, tbl->seed);;
    unsigned int index = hash % tbl->len;
    hashList *lst = tbl->table[index];
    
    if (lst == NULL) {
        tbl->table[index] = MEM_calloc(sizeof(hashList), __func__);
        lst = tbl->table[index];
        lst->element = element;
        lst->len = len;
        lst->name = name;
        lst->next = NULL;
        tbl->count ++;
        return;
    }
    lst = MEM_calloc(sizeof(hashList), __func__);
    lst->next = tbl->table[index];
    lst->name = name;
    lst->element = element;
    lst->len = len;
    
    tbl->table[index] = lst;
    tbl->count ++;
}

static void update_table(KER_hashTable *tbl)
{
    double load = (double) tbl->count / (double) tbl->len;
    
    if (load > 0.75) {
        tbl->oldtable = tbl->table;
        tbl->oldcount = tbl->count;
        tbl->oldlen = tbl->len;
        
        tbl->len = tbl->oldlen * 16;
        tbl->count = 0;
        tbl->table = (hashList **)MEM_calloc_array(tbl->len, sizeof(hashList*), __func__);
    }
    
    hashList **element;
    hashList *lst;
    if (tbl->oldcount > 0) {
        element = &tbl->oldtable[tbl->index];
        while (*element == NULL) {
            element++;
            tbl->index ++;
        }
        lst = *element;
        *element = lst->next;
        
        add_item(tbl, lst->name, lst->len, lst->element);
        MEM_free(lst);
        tbl->oldcount --;
    }
    
    if (tbl->oldcount == 0 && tbl->oldtable) {
        MEM_free(tbl->oldtable);
        tbl->oldtable = NULL;
        tbl->oldlen = 0;
        tbl->index = 0;
    }
}

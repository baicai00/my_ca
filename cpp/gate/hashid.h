#ifndef skynet_hashid_h
#define skynet_hashid_h

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct hashid_node {
    int id;
    struct hashid_node *next;
};

struct hashid {
    int hashmod;
    int cap;
    int count;
    struct hashid_node *id;
    struct hashid_node **hash;
};

static void
hashid_init(struct hashid *hi, int max) {
    int i;
    int hashcap = 16;
    while (hashcap < max) {
        hashcap *= 2;
    }

    hi->hashmod = hashcap - 1;
    hi->cap = max;
    hi->count = 0;
    hi->id = skynet_malloc(max * sizeof(struct hashid_node));
    for (i = 0; i < max; ++i) {
        hi->id[i].id = -1;
        hi->id[i].next = NULL;
    }
    hi->hash = skynet_malloc(hashcap * sizeof(struct hashid_node *));
    memset(hi->hash, 0, hashcap * sizeof(struct hashid_node *));
}

static void
hashid_clear(struct hashid *hi) {

}

static int
hashid_lookup(struct hashid *hi, int id) {

}

static int
hashid_remove(struct hashid *hi, int id) {

}

static int
hashid_insert(struct hashid * hi, int id) {

}

static inline int
hashid_full(struct hashid *hi) {

}

#endif

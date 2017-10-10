#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

#define KEY_LENGTH 1024

typedef struct _node_t {
    char key[KEY_LENGTH];
    void *data;
    struct _node_t *next;
} node_t;

struct _hashtable_t {
    size_t expired;
    size_t size;
    node_t **arr;
};

static unsigned long _hash(const char *str);
static void _free_node(node_t *node);

hashtable_t *hsh_init(size_t size, size_t expired)
{
    hashtable_t *tbl = NULL;
    tbl = (hashtable_t *)calloc(1, sizeof(hashtable_t));
    if (!tbl) {
        printf("ERROR: Cannot allocate memory for hashtable\n");
        goto done;
    }
    tbl->expired = expired;
    tbl->size = size;
    tbl->arr = (node_t **)calloc(size, sizeof(node_t *));
    if (!tbl->arr) {
        printf("ERROR: Cannot allocate memory for hashtable nodes\n");
        free(tbl);
        goto done;
    }
done:
    return tbl;
}

int hsh_put(hashtable_t *tbl, const char *key, void *data)
{
    int status = 0;

    /* Create new node to insert */
    node_t *newnode = (node_t *)calloc(1, sizeof(node_t));
    if (!newnode) {
        goto done;
    }
    strncpy(newnode->key, key, KEY_LENGTH);
    newnode->data = data;

    unsigned long index = _hash(key) % tbl->size;
    if (!tbl->arr[index]) {
        tbl->arr[index] = newnode;
    }
    else {
        node_t *ptr = tbl->arr[index];
        do {
            if (strcmp(ptr->key, key) == 0) {
                status = -1;    /* Key already existed */
                _free_node(newnode);
                goto done;
            }
            ptr = ptr->next;
        } while (ptr->next);
        ptr->next = newnode;
    }
done:
    return status;
}

void *hsh_search(hashtable_t *tbl, const char *key)
{
    void *data = NULL;
    unsigned long index = _hash(key) % tbl->size;
    node_t *ptr = tbl->arr[index];
    while (ptr) {
        if (strcmp(ptr->key, key) == 0) {
            data = ptr->data;
            goto done;
        }
        ptr = ptr->next;
    }
done:
    return data;
}

int hsh_remove(hashtable_t *tbl, const char *key)
{
    node_t *to_be_removed = NULL;
    unsigned long index = _hash(key) % tbl->size;
    if (tbl->arr[index] && strcmp(tbl->arr[index]->key, key) == 0) {
        to_be_removed = tbl->arr[index];
        tbl->arr[index] = tbl->arr[index]->next;
    }
    else if (tbl->arr[index]) {
        node_t *ptr = tbl->arr[index];
        while (ptr->next) {
            if (strcmp(ptr->next->key, key) == 0) {
                to_be_removed = ptr->next;
                ptr->next = ptr->next->next;
                break;
            }
            ptr = ptr->next;
        }
    }
    if (to_be_removed) {
        _free_node(to_be_removed);
        return 0;
    }
    else {
        return -1;
    }
}

int hsh_replace(hashtable_t *tbl, const char *key, void *newdata)
{
    unsigned long index = _hash(key) % tbl->size;
    node_t *ptr = tbl->arr[index];
    node_t *to_be_replaced = NULL;
    while (ptr) {
        if (strcmp(ptr->key, key) == 0) {
            free(ptr->data);
            ptr->data = newdata;
            return 0;
        }
        ptr = ptr->next;
    }
    free(newdata);
    return -1;
}

void hsh_destroy(hashtable_t *tbl)
{
    int i;
    for (i = 0; i < tbl->size; i++) {
        if (tbl->arr[i]) {
            while (tbl->arr[i]) {
                node_t *node = tbl->arr[i];
                tbl->arr[i] = tbl->arr[i]->next;
                _free_node(node);
            }
        }
    }
    free(tbl->arr);
    free(tbl);
}

static unsigned long _hash(const char *str)
{
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static void _free_node(node_t *node)
{
    if (node->data) free(node->data);
    free(node);
}

#ifndef __FCLEANER_HASHTABLE_H__
#define __FCLEANER_HASHTABLE_H__

typedef struct _hashtable_t hashtable_t;

/**
  * @brief Init hashtable
  * @param size Size of hashtable
  * @param expired Expired time of item, expired = 0 : no expiry
  * @return Pointer to hashtable if success, NULL if failed
  */
hashtable_t *hsh_init(size_t size, size_t expired);

/**
  * @brief Add an item to hashtable, if item exists, it will be destroyed and replaced by new item
  * @param tbl Pointer to hashtable
  * @param key String key
  * @param data Pointer to data
  * @return 0 if success
  */
int hsh_put(hashtable_t *tbl, const char *key, void *data);

/**
  * @brief Search for item in hashtable from key
  * @param tbl Pointer to hashtable
  * @param key String key
  * @return Item if found
  */
void *hsh_search(hashtable_t *tbl, const char *key);

/**
  * @brief Remove an item from hashtable
  * @param tbl Pointer to hashtable
  * @param key String key
  * @return 0 if remove success, other if fail (item not exist ...)
  */
int hsh_remove(hashtable_t *tbl, const char *key);

/**
  * @brief Replace an item by another one
  * @param tbl Pointer to hashtable
  * @param key String key
  * @param newdata New data to be replaced
  * @return 0 if replace success, other if failed (item not exist ...)
  */
int hsh_replace(hashtable_t *tbl, const char *key, void *newdata);

/**
  * @brief Destroy hastable, release all items
  * @param tbl Pointer to hashtable
  */
void hsh_destroy(hashtable_t *tbl);

#endif

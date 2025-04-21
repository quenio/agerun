#ifndef AGERUN_LIST_H
#define AGERUN_LIST_H

#include <stdbool.h>
#include <stddef.h>

/**
 * A linked list structure for storing pointer items.
 * The list never owns or manages the memory for items.
 * The caller is always responsible for allocating and freeing memory for all items.
 */
typedef struct list_s list_t;

/**
 * Create a new empty list
 * @return Pointer to the new list, or NULL on failure
 */
list_t* ar_list_create(void);

/**
 * Add an item to the end of the list
 * @param list The list to add to
 * @param item The item to add
 * @return true if successful, false otherwise
 */
bool ar_list_add_last(list_t *list, void *item);

/**
 * Add an item to the beginning of the list
 * @param list The list to add to
 * @param item The item to add
 * @return true if successful, false otherwise
 */
bool ar_list_add_first(list_t *list, void *item);

/**
 * Get the first item in the list
 * @param list The list to get the first item from
 * @return Pointer to the first item, or NULL if the list is empty
 */
void* ar_list_first(const list_t *list);

/**
 * Get the last item in the list
 * @param list The list to get the last item from
 * @return Pointer to the last item, or NULL if the list is empty
 */
void* ar_list_last(const list_t *list);

/**
 * Remove and return the first item from the list
 * @param list The list to remove from
 * @return Pointer to the removed item, or NULL if the list is empty
 */
void* ar_list_remove_first(list_t *list);

/**
 * Remove and return the last item from the list
 * @param list The list to remove from
 * @return Pointer to the removed item, or NULL if the list is empty
 */
void* ar_list_remove_last(list_t *list);

/**
 * Get the number of items in the list
 * @param list The list to count
 * @return The number of items
 */
size_t ar_list_count(const list_t *list);

/**
 * Check if the list is empty
 * @param list The list to check
 * @return true if the list is empty, false otherwise
 */
bool ar_list_empty(const list_t *list);

/**
 * Get an array of all items in the list
 * @param list The list to get items from
 * @return Array of pointers to items, or NULL on failure
 * @note The caller is responsible for freeing the returned array using free().
 *       The items themselves are not copied and remain owned by the caller.
 *       The caller can use ar_list_count() to determine the size of the array.
 */
void** ar_list_items(const list_t *list);

/**
 * Free all resources in a list
 * @param list List to free
 * @note This function only frees the list structure itself.
 *       It does not free memory for items.
 *       The caller is responsible for freeing all items that were added to the list.
 */
void ar_list_destroy(list_t *list);

#endif /* AGERUN_LIST_H */

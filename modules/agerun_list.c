#include "agerun_list.h"
#include <stdlib.h>

/**
 * List node structure
 */
struct list_node_s {
    void *item;                   // Pointer to the item
    struct list_node_s *next;     // Pointer to the next node
};

/**
 * List structure
 */
struct list_s {
    struct list_node_s *head;     // Pointer to the first node
    struct list_node_s *tail;     // Pointer to the last node
    size_t count;                 // Number of items in the list
};

/**
 * Create a new empty list
 * @return Pointer to the new list, or NULL on failure
 */
list_t* ar_list_create(void) {
    list_t *list = (list_t*)malloc(sizeof(list_t));
    if (!list) {
        return NULL;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    
    return list;
}

/**
 * Append an item to the end of the list
 * @param list The list to append to
 * @param item The item to append
 * @return true if successful, false otherwise
 */
bool ar_list_append(list_t *list, void *item) {
    if (!list) {
        return false;
    }
    
    struct list_node_s *node = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    if (!node) {
        return false;
    }
    
    node->item = item;
    node->next = NULL;
    
    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    
    list->tail = node;
    list->count++;
    
    return true;
}

/**
 * Get the number of items in the list
 * @param list The list to count
 * @return The number of items
 */
size_t ar_list_count(const list_t *list) {
    if (!list) {
        return 0;
    }
    
    return list->count;
}

/**
 * Check if the list is empty
 * @param list The list to check
 * @return true if the list is empty, false otherwise
 */
bool ar_list_empty(const list_t *list) {
    if (!list) {
        return true;
    }
    
    return list->count == 0;
}

/**
 * Get an array of all items in the list
 * @param list The list to get items from
 * @return Array of pointers to items, or NULL on failure
 * @note The caller is responsible for freeing the returned array using free().
 *       The items themselves are not copied and remain owned by the caller.
 *       The caller can use ar_list_count() to determine the size of the array.
 */
void** ar_list_items(const list_t *list) {
    if (!list) {
        return NULL;
    }
    
    if (list->count == 0) {
        return NULL;
    }
    
    void **items = (void**)malloc(list->count * sizeof(void*));
    if (!items) {
        return NULL;
    }
    
    struct list_node_s *current = list->head;
    size_t index = 0;
    
    while (current) {
        items[index++] = current->item;
        current = current->next;
    }
    
    return items;
}

/**
 * Free all resources in a list
 * @param list List to free
 * @note This function only frees the list structure itself.
 *       It does not free memory for items.
 *       The caller is responsible for freeing all items that were added to the list.
 */
void ar_list_destroy(list_t *list) {
    if (!list) {
        return;
    }
    
    struct list_node_s *current = list->head;
    while (current) {
        struct list_node_s *next = current->next;
        free(current);
        current = next;
    }
    
    free(list);
}

#include "agerun_list.h"
#include <stdlib.h>

/**
 * List node structure
 */
struct list_node_s {
    void *item;                   // Pointer to the item
    struct list_node_s *next;     // Pointer to the next node
    struct list_node_s *prev;     // Pointer to the previous node
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
 * Add an item to the end of the list
 * @param list The list to add to
 * @param item The item to add
 * @return true if successful, false otherwise
 */
bool ar_list_add_last(list_t *list, void *item) {
    if (!list) {
        return false;
    }
    
    struct list_node_s *node = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    if (!node) {
        return false;
    }
    
    node->item = item;
    node->next = NULL;
    node->prev = list->tail;   // Set the previous pointer to current tail
    
    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;     // If the list was empty, also set the head
    }
    
    list->tail = node;
    list->count++;
    
    return true;
}

/**
 * Add an item to the beginning of the list
 * @param list The list to add to
 * @param item The item to add
 * @return true if successful, false otherwise
 */
bool ar_list_add_first(list_t *list, void *item) {
    if (!list) {
        return false;
    }
    
    struct list_node_s *node = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    if (!node) {
        return false;
    }
    
    node->item = item;
    node->next = list->head;
    node->prev = NULL;         // First node has no previous
    
    if (list->head) {
        list->head->prev = node;  // Update the previous head's prev pointer
    } else {
        list->tail = node;     // If the list was empty, also set the tail
    }
    
    list->head = node;
    list->count++;
    
    return true;
}

/**
 * Get the first item in the list
 * @param list The list to get the first item from
 * @return Pointer to the first item, or NULL if the list is empty
 */
void* ar_list_first(const list_t *list) {
    if (!list || !list->head) {
        return NULL;
    }
    
    return list->head->item;
}

/**
 * Get the last item in the list
 * @param list The list to get the last item from
 * @return Pointer to the last item, or NULL if the list is empty
 */
void* ar_list_last(const list_t *list) {
    if (!list || !list->tail) {
        return NULL;
    }
    
    return list->tail->item;
}

/**
 * Remove and return the first item from the list
 * @param list The list to remove from
 * @return Pointer to the removed item, or NULL if the list is empty
 */
void* ar_list_remove_first(list_t *list) {
    if (!list || !list->head) {
        return NULL;
    }
    
    struct list_node_s *node = list->head;
    void *item = node->item;
    
    list->head = node->next;
    
    if (list->head) {
        list->head->prev = NULL;  // Update new head to have no previous
    } else {
        list->tail = NULL;       // If list is now empty, update tail too
    }
    
    free(node);
    list->count--;
    
    return item;
}

/**
 * Remove and return the last item from the list
 * @param list The list to remove from
 * @return Pointer to the removed item, or NULL if the list is empty
 */
void* ar_list_remove_last(list_t *list) {
    if (!list || !list->tail) {
        return NULL;
    }
    
    struct list_node_s *node = list->tail;
    void *item = node->item;
    
    list->tail = node->prev;  // Update tail to the previous node
    
    if (list->tail) {
        list->tail->next = NULL;  // New tail has no next
    } else {
        list->head = NULL;        // If list is now empty, update head too
    }
    
    free(node);
    list->count--;
    
    return item;
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

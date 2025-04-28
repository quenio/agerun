#include "agerun_list.h"
#include "agerun_debug.h"
#include <stdlib.h>

/**
 * List node structure
 */
struct list_node_s {
    void *ref_item;              // Pointer to the item (borrowed reference)
    struct list_node_s *mut_next; // Pointer to the next node (mutable reference)
    struct list_node_s *mut_prev; // Pointer to the previous node (mutable reference)
};

/**
 * List structure
 */
struct list_s {
    struct list_node_s *own_head; // Pointer to the first node (owned by list)
    struct list_node_s *own_tail; // Pointer to the last node (owned by list)
    size_t count;                 // Number of items in the list
};

/**
 * Create a new empty list
 * @return Pointer to the new list, or NULL on failure
 */
list_t* ar_list_create(void) {
    list_t *own_list = (list_t*)malloc(sizeof(list_t));
    if (!own_list) {
        return NULL;
    }
    
    own_list->own_head = NULL;
    own_list->own_tail = NULL;
    own_list->count = 0;
    
    return own_list; // Ownership transferred to caller
}

/**
 * Add an item to the end of the list
 * @param list The list to add to
 * @param item The item to add
 * @return true if successful, false otherwise
 */
bool ar_list_add_last(list_t *mut_list, void *ref_item) {
    if (!mut_list) {
        return false;
    }
    
    struct list_node_s *own_node = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    if (!own_node) {
        return false;
    }
    
    own_node->ref_item = ref_item;
    own_node->mut_next = NULL;
    own_node->mut_prev = mut_list->own_tail;   // Set the previous pointer to current tail
    
    if (mut_list->own_tail) {
        mut_list->own_tail->mut_next = own_node;
    } else {
        mut_list->own_head = own_node;     // If the list was empty, also set the head
    }
    
    mut_list->own_tail = own_node;
    mut_list->count++;
    
    return true;
}

/**
 * Add an item to the beginning of the list
 * @param list The list to add to
 * @param item The item to add
 * @return true if successful, false otherwise
 */
bool ar_list_add_first(list_t *mut_list, void *ref_item) {
    if (!mut_list) {
        return false;
    }
    
    struct list_node_s *own_node = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    if (!own_node) {
        return false;
    }
    
    own_node->ref_item = ref_item;
    own_node->mut_next = mut_list->own_head;
    own_node->mut_prev = NULL;         // First node has no previous
    
    if (mut_list->own_head) {
        mut_list->own_head->mut_prev = own_node;  // Update the previous head's prev pointer
    } else {
        mut_list->own_tail = own_node;     // If the list was empty, also set the tail
    }
    
    mut_list->own_head = own_node;
    mut_list->count++;
    
    return true;
}

/**
 * Get the first item in the list
 * @param list The list to get the first item from
 * @return Pointer to the first item, or NULL if the list is empty
 */
void* ar_list_first(const list_t *ref_list) {
    if (!ref_list || !ref_list->own_head) {
        return NULL;
    }
    
    return ref_list->own_head->ref_item; // Borrowed reference, not owned by caller
}

/**
 * Get the last item in the list
 * @param list The list to get the last item from
 * @return Pointer to the last item, or NULL if the list is empty
 */
void* ar_list_last(const list_t *ref_list) {
    if (!ref_list || !ref_list->own_tail) {
        return NULL;
    }
    
    return ref_list->own_tail->ref_item; // Borrowed reference, not owned by caller
}

/**
 * Remove and return the first item from the list
 * @param list The list to remove from
 * @return Pointer to the removed item, or NULL if the list is empty
 */
void* ar_list_remove_first(list_t *mut_list) {
    if (!mut_list || !mut_list->own_head) {
        return NULL;
    }
    
    struct list_node_s *mut_node = mut_list->own_head;
    void *ref_item = mut_node->ref_item;
    
    mut_list->own_head = mut_node->mut_next;
    
    if (mut_list->own_head) {
        mut_list->own_head->mut_prev = NULL;  // Update new head to have no previous
    } else {
        mut_list->own_tail = NULL;       // If list is now empty, update tail too
    }
    
    free(mut_node);
    // Note: Setting mut_node to NULL is technically not needed as it's a local variable
    // but it helps signal that the memory is no longer accessible.
    mut_list->count--;
    
    return ref_item; // Borrowed reference, ownership not transferred
}

/**
 * Remove and return the last item from the list
 * @param list The list to remove from
 * @return Pointer to the removed item, or NULL if the list is empty
 */
void* ar_list_remove_last(list_t *mut_list) {
    if (!mut_list || !mut_list->own_tail) {
        return NULL;
    }
    
    struct list_node_s *mut_node = mut_list->own_tail;
    void *ref_item = mut_node->ref_item;
    
    mut_list->own_tail = mut_node->mut_prev;  // Update tail to the previous node
    
    if (mut_list->own_tail) {
        mut_list->own_tail->mut_next = NULL;  // New tail has no next
    } else {
        mut_list->own_head = NULL;        // If list is now empty, update head too
    }
    
    free(mut_node);
    // Note: Setting mut_node to NULL is technically not needed as it's a local variable
    // but it helps signal that the memory is no longer accessible.
    mut_list->count--;
    
    return ref_item; // Borrowed reference, ownership not transferred
}

/**
 * Get the number of items in the list
 * @param list The list to count
 * @return The number of items
 */
size_t ar_list_count(const list_t *ref_list) {
    if (!ref_list) {
        return 0;
    }
    
    return ref_list->count;
}

/**
 * Check if the list is empty
 * @param list The list to check
 * @return true if the list is empty, false otherwise
 */
bool ar_list_empty(const list_t *ref_list) {
    if (!ref_list) {
        return true;
    }
    
    return ref_list->count == 0;
}

/**
 * Get an array of all items in the list
 * @param list The list to get items from
 * @return Array of pointers to items, or NULL on failure
 * @note The caller is responsible for freeing the returned array using free().
 *       The items themselves are not copied and remain owned by the caller.
 *       The caller can use ar_list_count() to determine the size of the array.
 */
void** ar_list_items(const list_t *ref_list) {
    if (!ref_list) {
        return NULL;
    }
    
    if (ref_list->count == 0) {
        return NULL;
    }
    
    void **own_items = (void**)malloc(ref_list->count * sizeof(void*));
    if (!own_items) {
        return NULL;
    }
    
    struct list_node_s *ref_current = ref_list->own_head;
    size_t index = 0;
    
    while (ref_current) {
        own_items[index++] = ref_current->ref_item;
        ref_current = ref_current->mut_next;
    }
    
    return own_items; // Ownership of array transferred to caller, but not items
}

/**
 * Remove all occurrences of an item from the list by value
 * @param list The list to remove from
 * @param item The item to remove
 * @return true if at least one occurrence of the item was found and removed, false otherwise
 * @note This function compares the item pointer directly with the stored pointers,
 *       not the contents of what they point to.
 */
bool ar_list_remove(list_t *mut_list, void *ref_item) {
    if (!mut_list || mut_list->count == 0) {
        return false;
    }
    
    bool found = false;
    struct list_node_s *mut_current = mut_list->own_head;
    
    while (mut_current) {
        struct list_node_s *mut_next = mut_current->mut_next;
        
        // Check if this node contains the item to remove
        if (mut_current->ref_item == ref_item) {
            // Remove this node from the list
            
            // Update the previous node's next pointer (or head if this is the first node)
            if (mut_current->mut_prev) {
                mut_current->mut_prev->mut_next = mut_current->mut_next;
            } else {
                mut_list->own_head = mut_current->mut_next;
            }
            
            // Update the next node's previous pointer (or tail if this is the last node)
            if (mut_current->mut_next) {
                mut_current->mut_next->mut_prev = mut_current->mut_prev;
            } else {
                mut_list->own_tail = mut_current->mut_prev;
            }
            
            // Free the node
            free(mut_current);
            // Note: Setting mut_current to NULL is technically not needed as we don't use it after this,
            // but it would help signal that the memory is no longer accessible in more complex contexts.
            
            // Decrement count
            mut_list->count--;
            
            // Mark that we found at least one occurrence
            found = true;
        }
        
        mut_current = mut_next;
    }
    
    return found;
}

/**
 * Free all resources in a list
 * @param list List to free
 * @note This function only frees the list structure itself.
 *       It does not free memory for items.
 *       The caller is responsible for freeing all items that were added to the list.
 */
void ar_list_destroy(list_t *own_list) {
    if (!own_list) {
        return;
    }
    
    struct list_node_s *mut_current = own_list->own_head;
    while (mut_current) {
        struct list_node_s *mut_next = mut_current->mut_next;
        free(mut_current);
        // Move to next node
        mut_current = mut_next;
    }
    
    free(own_list);
    // Note: Setting own_list to NULL here doesn't affect the caller's variable,
    // since C passes parameters by value. We rely on the caller to not use the 
    // pointer after calling this function.
}

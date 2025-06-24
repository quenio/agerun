#include "ar_list.h"
#include "ar_heap.h"
#include <stdlib.h>

/**
 * List node structure
 */
struct list_node_s {
    void *mut_item;              // Pointer to the item (mutable borrowed reference)
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
list_t* ar__list__create(void) {
    list_t *own_list = (list_t*)AR__HEAP__MALLOC(sizeof(list_t), "List structure");
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
bool ar__list__add_last(list_t *mut_list, void *mut_item) {
    if (!mut_list) {
        return false;
    }
    
    struct list_node_s *own_node = (struct list_node_s*)AR__HEAP__MALLOC(sizeof(struct list_node_s), "List node");
    if (!own_node) {
        return false;
    }
    
    own_node->mut_item = mut_item;
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
bool ar__list__add_first(list_t *mut_list, void *mut_item) {
    if (!mut_list) {
        return false;
    }
    
    struct list_node_s *own_node = (struct list_node_s*)AR__HEAP__MALLOC(sizeof(struct list_node_s), "List node");
    if (!own_node) {
        return false;
    }
    
    own_node->mut_item = mut_item;
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
void* ar__list__first(const list_t *ref_list) {
    if (!ref_list || !ref_list->own_head) {
        return NULL;
    }
    
    return ref_list->own_head->mut_item; // Borrowed reference, not owned by caller
}

/**
 * Get the last item in the list
 * @param list The list to get the last item from
 * @return Pointer to the last item, or NULL if the list is empty
 */
void* ar__list__last(const list_t *ref_list) {
    if (!ref_list || !ref_list->own_tail) {
        return NULL;
    }
    
    return ref_list->own_tail->mut_item; // Borrowed reference, not owned by caller
}

/**
 * Remove and return the first item from the list
 * @param list The list to remove from
 * @return Pointer to the removed item, or NULL if the list is empty
 */
void* ar__list__remove_first(list_t *mut_list) {
    if (!mut_list || !mut_list->own_head) {
        return NULL;
    }
    
    struct list_node_s *mut_node = mut_list->own_head;
    void *mut_item = mut_node->mut_item;
    
    mut_list->own_head = mut_node->mut_next;
    
    if (mut_list->own_head) {
        mut_list->own_head->mut_prev = NULL;  // Update new head to have no previous
    } else {
        mut_list->own_tail = NULL;       // If list is now empty, update tail too
    }
    
    AR__HEAP__FREE(mut_node);
    // Note: Setting mut_node to NULL is technically not needed as it's a local variable
    // but it helps signal that the memory is no longer accessible.
    mut_list->count--;
    
    return mut_item; // Borrowed reference, ownership not transferred
}

/**
 * Remove and return the last item from the list
 * @param list The list to remove from
 * @return Pointer to the removed item, or NULL if the list is empty
 */
void* ar__list__remove_last(list_t *mut_list) {
    if (!mut_list || !mut_list->own_tail) {
        return NULL;
    }
    
    struct list_node_s *mut_node = mut_list->own_tail;
    void *mut_item = mut_node->mut_item;
    
    mut_list->own_tail = mut_node->mut_prev;  // Update tail to the previous node
    
    if (mut_list->own_tail) {
        mut_list->own_tail->mut_next = NULL;  // New tail has no next
    } else {
        mut_list->own_head = NULL;        // If list is now empty, update head too
    }
    
    AR__HEAP__FREE(mut_node);
    // Note: Setting mut_node to NULL is technically not needed as it's a local variable
    // but it helps signal that the memory is no longer accessible.
    mut_list->count--;
    
    return mut_item; // Borrowed reference, ownership not transferred
}

/**
 * Get the number of items in the list
 * @param list The list to count
 * @return The number of items
 */
size_t ar__list__count(const list_t *ref_list) {
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
bool ar__list__empty(const list_t *ref_list) {
    if (!ref_list) {
        return true;
    }
    
    return ref_list->count == 0;
}

/**
 * Get an array of all items in the list
 * @param list The list to get items from
 * @return Array of pointers to items, or NULL on failure
 * @note The caller is responsible for freeing the returned array using AR__HEAP__FREE().
 *       The items themselves are not copied and remain owned by the caller.
 *       The caller can use ar_list_count() to determine the size of the array.
 */
void** ar__list__items(const list_t *ref_list) {
    if (!ref_list) {
        return NULL;
    }
    
    if (ref_list->count == 0) {
        return NULL;
    }
    
    void **own_items = (void**)AR__HEAP__MALLOC(ref_list->count * sizeof(void*), "List items array");
    if (!own_items) {
        return NULL;
    }
    
    struct list_node_s *ref_current = ref_list->own_head;
    size_t index = 0;
    
    while (ref_current) {
        own_items[index++] = ref_current->mut_item;
        ref_current = ref_current->mut_next;
    }
    
    return own_items; // Ownership of array transferred to caller, but not items
}

/**
 * Remove the first occurrence of an item from the list by value
 * @param mut_list The list to remove from (mutable reference)
 * @param ref_item The item to remove (const borrowed reference)
 * @return The removed item as a non-const pointer, or NULL if it was not found
 * @note Ownership: This function does not affect ownership of the item.
 *       The caller remains responsible for freeing the item if necessary.
 * @note This function compares the item pointer directly with the stored pointers,
 *       not the contents of what they point to.
 */
void* ar__list__remove(list_t *mut_list, const void *ref_item) {
    if (!mut_list || mut_list->count == 0) {
        return NULL;
    }
    
    struct list_node_s *mut_current = mut_list->own_head;
    
    while (mut_current) {
        // Check if this node contains the item to remove
        if (mut_current->mut_item == ref_item) {
            // Store the item to return
            void *removed_item = (void*)mut_current->mut_item;
            
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
            AR__HEAP__FREE(mut_current);
            
            // Decrement count
            mut_list->count--;
            
            // Return the removed item (with const qualifier removed)
            return removed_item;
        }
        
        mut_current = mut_current->mut_next;
    }
    
    return NULL;
}

/**
 * Free all resources in a list
 * @param list List to free
 * @note This function only frees the list structure itself.
 *       It does not free memory for items.
 *       The caller is responsible for freeing all items that were added to the list.
 */
void ar__list__destroy(list_t *own_list) {
    if (!own_list) {
        return;
    }
    
    struct list_node_s *mut_current = own_list->own_head;
    while (mut_current) {
        struct list_node_s *mut_next = mut_current->mut_next;
        AR__HEAP__FREE(mut_current);
        // Move to next node
        mut_current = mut_next;
    }
    
    AR__HEAP__FREE(own_list);
    // Note: Setting own_list to NULL here doesn't affect the caller's variable,
    // since C passes parameters by value. We rely on the caller to not use the 
    // pointer after calling this function.
}

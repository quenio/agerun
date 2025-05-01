#ifndef AGERUN_LIST_H
#define AGERUN_LIST_H

#include <stdbool.h>
#include <stddef.h>

/**
 * A linked list structure for storing pointer items.
 * The list never owns or manages the memory for items.
 * The caller is always responsible for allocating and freeing memory for all items.
 * 
 * This module follows the AgeRun Memory Management Model (MMM):
 * - The list structure itself is an owned value that must be destroyed by its owner
 * - The list never takes ownership of stored items - they remain owned by the caller
 * - Functions like ar_list_first() return borrowed references that should not be destroyed
 * - The ar_list_items() function transfers ownership of the returned array but not the items
 */
typedef struct list_s list_t;

/**
 * Create a new empty list
 * @return Pointer to the new list, or NULL on failure
 * @note Ownership: Returns an owned value that the caller must eventually destroy using ar_list_destroy()
 */
list_t* ar_list_create(void);

/**
 * Add an item to the end of the list
 * @param mut_list The list to add to (mutable reference)
 * @param ref_item The item to add (borrowed reference)
 * @return true if successful, false otherwise
 * @note Ownership: Borrows the item without taking ownership. The caller remains responsible for the item's memory.
 */
bool ar_list_add_last(list_t *mut_list, void *ref_item);

/**
 * Add an item to the beginning of the list
 * @param mut_list The list to add to (mutable reference)
 * @param ref_item The item to add (borrowed reference)
 * @return true if successful, false otherwise
 * @note Ownership: Borrows the item without taking ownership. The caller remains responsible for the item's memory.
 */
bool ar_list_add_first(list_t *mut_list, void *ref_item);

/**
 * Get the first item in the list
 * @param ref_list The list to get the first item from (borrowed reference)
 * @return Pointer to the first item, or NULL if the list is empty
 * @note Ownership: Returns a borrowed reference. The caller must not destroy the returned item.
 */
void* ar_list_first(const list_t *ref_list);

/**
 * Get the last item in the list
 * @param ref_list The list to get the last item from (borrowed reference)
 * @return Pointer to the last item, or NULL if the list is empty
 * @note Ownership: Returns a borrowed reference. The caller must not destroy the returned item.
 */
void* ar_list_last(const list_t *ref_list);

/**
 * Remove and return the first item from the list
 * @param mut_list The list to remove from (mutable reference)
 * @return Pointer to the removed item, or NULL if the list is empty
 * @note Ownership: Returns a borrowed reference. The list does not transfer ownership to the caller.
 */
void* ar_list_remove_first(list_t *mut_list);

/**
 * Remove and return the last item from the list
 * @param mut_list The list to remove from (mutable reference)
 * @return Pointer to the removed item, or NULL if the list is empty
 * @note Ownership: Returns a borrowed reference. The list does not transfer ownership to the caller.
 */
void* ar_list_remove_last(list_t *mut_list);

/**
 * Get the number of items in the list
 * @param ref_list The list to count (borrowed reference)
 * @return The number of items
 * @note Ownership: No ownership implications; this is a pure query operation.
 */
size_t ar_list_count(const list_t *ref_list);

/**
 * Check if the list is empty
 * @param ref_list The list to check (borrowed reference)
 * @return true if the list is empty, false otherwise
 * @note Ownership: No ownership implications; this is a pure query operation.
 */
bool ar_list_empty(const list_t *ref_list);

/**
 * Get an array of all items in the list
 * @param ref_list The list to get items from (borrowed reference)
 * @return Array of pointers to items, or NULL on failure
 * @note Ownership: Transfers ownership of the returned array to the caller, who must free it using free().
 *       The items in the array remain borrowed references.
 *       The caller can use ar_list_count() to determine the size of the array.
 */
void** ar_list_items(const list_t *ref_list);

/**
 * Remove the first occurrence of an item from the list by value
 * @param mut_list The list to remove from (mutable reference)
 * @param ref_item The item to remove (const borrowed reference)
 * @return The removed item as a non-const pointer, or NULL if it was not found
 * @note Ownership: This function does not affect ownership of the item. 
 *       The caller remains responsible for freeing the item if necessary.
 * @note This function compares the item pointer directly with the stored pointers,
 *       not the contents of what they point to.
 * @note This function accepts a const reference to the item to facilitate removing
 *       items from the list that have been passed as const references, particularly
 *       when transferring ownership from a context to a caller.
 * @note When the item is removed, it is returned as a non-const pointer, allowing the
 *       caller to take ownership of it regardless of how it was originally passed.
 */
void* ar_list_remove(list_t *mut_list, const void *ref_item);

/**
 * Free all resources in a list
 * @param own_list List to free (owned value)
 * @note Ownership: Takes ownership of the list parameter.
 *       This function only frees the list structure itself.
 *       It does not free memory for items.
 *       The caller is responsible for freeing all items that were added to the list.
 */
void ar_list_destroy(list_t *own_list);

#endif /* AGERUN_LIST_H */

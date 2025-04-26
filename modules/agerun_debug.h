#ifndef AGERUN_DEBUG_H
#define AGERUN_DEBUG_H

#include <assert.h>

/**
 * Debug utilities for the AgeRun system.
 * 
 * This module provides macros and utilities to help enforce various
 * invariants and detect problems during development.
 */

/**
 * Helper macro for checking ownership invariants in debug builds.
 * In release builds, this becomes a no-op.
 * 
 * Use this to verify that pointers that should never be NULL (like owned
 * values after creation) actually have valid values.
 */
#ifdef DEBUG
#define AR_ASSERT_OWNERSHIP(ptr) assert((ptr) != NULL && "Ownership violation: NULL pointer")
#else
#define AR_ASSERT_OWNERSHIP(ptr) ((void)0)
#endif

/**
 * Helper macro for validating that a pointer is NULL after ownership transfer.
 * In release builds, this becomes a no-op.
 * 
 * Use this to verify that pointers have been properly set to NULL after
 * their ownership has been transferred to another function or container.
 */
#ifdef DEBUG
#define AR_ASSERT_TRANSFERRED(ptr) assert((ptr) == NULL && "Ownership violation: Pointer not NULL after transfer")
#else
#define AR_ASSERT_TRANSFERRED(ptr) ((void)0)
#endif

/**
 * Helper macro for checking that a pointer is not used after being freed.
 * In release builds, this becomes a no-op.
 * 
 * This is particularly useful for local variables that are destroyed and
 * should not be accessed afterward.
 */
#ifdef DEBUG
#define AR_ASSERT_NOT_USED_AFTER_FREE(ptr) assert((ptr) == NULL && "Usage after free: Pointer accessed after being freed")
#else
#define AR_ASSERT_NOT_USED_AFTER_FREE(ptr) ((void)0)
#endif

#endif /* AGERUN_DEBUG_H */
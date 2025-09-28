#ifndef AGERUN_AGENT_STORE_H
#define AGERUN_AGENT_STORE_H

#include <stdbool.h>
#include <stdint.h>

/* Forward declarations */
typedef struct ar_agent_registry_s ar_agent_registry_t;
typedef struct ar_agent_store_s ar_agent_store_t;
typedef struct ar_methodology_s ar_methodology_t;

/**
 * @file ar_agent_store.h
 * @brief Agent store module for persisting and loading agent state
 * 
 * This module handles saving and loading agent state to/from persistent storage.
 * It manages the agerun.agency file format and ensures data integrity during
 * persistence operations.
 */

/* Constants */
#define AGENT_STORE_FILE_NAME "agerun.agency"
#define AGENT_STORE_BACKUP_EXT ".bak"

/**
 * Create a new agent store instance
 * @param ref_registry The agent registry to work with (borrowed reference)
 * @param ref_methodology The methodology for method lookups (borrowed reference)
 * @return New agent store instance (ownership transferred), or NULL on failure
 * @note Ownership: Caller must destroy the returned agent store
 */
ar_agent_store_t* ar_agent_store__create(ar_agent_registry_t *ref_registry, ar_methodology_t *ref_methodology);

/**
 * Destroy an agent store instance
 * @param own_store The agent store to destroy (ownership transferred)
 * @note Ownership: Destroys the agent store instance
 */
void ar_agent_store__destroy(ar_agent_store_t *own_store);

/**
 * Save all agents to persistent storage
 * @param ref_store The agent store instance (borrowed reference)
 * @return true if successful, false otherwise
 * @note Creates a backup of existing file before saving
 * @note Ownership: Does not affect ownership of any agent resources
 */
bool ar_agent_store__save(ar_agent_store_t *ref_store);

/**
 * Load all agents from persistent storage
 * @param mut_store The agent store instance (mutable reference)
 * @return true if successful, false otherwise
 * @note Creates agents with their saved state
 * @note Ownership: Creates new agents with their own resources
 */
bool ar_agent_store__load(ar_agent_store_t *mut_store);

/**
 * Check if agent store file exists
 * @param ref_store The agent store instance (borrowed reference)
 * @return true if store file exists, false otherwise
 */
bool ar_agent_store__exists(ar_agent_store_t *ref_store);

/**
 * Delete the agent store file
 * @param ref_store The agent store instance (borrowed reference)
 * @return true if successful or file didn't exist, false on error
 * @note Creates a backup before deletion
 */
bool ar_agent_store__delete(ar_agent_store_t *ref_store);

/**
 * Get the path to the agent store file
 * @param ref_store The agent store instance (borrowed reference)
 * @return Path to the store file (constant string)
 */
const char* ar_agent_store__get_path(ar_agent_store_t *ref_store);

/**
 * Get the methodology reference from agent store
 * @param ref_store The agent store instance (borrowed reference)
 * @return Methodology reference (borrowed), or NULL if not set
 */
ar_methodology_t* ar_agent_store__get_methodology(ar_agent_store_t *ref_store);

#endif /* AGERUN_AGENT_STORE_H */
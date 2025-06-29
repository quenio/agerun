#ifndef AGERUN_AGENT_STORE_H
#define AGERUN_AGENT_STORE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file agerun_agent_store.h
 * @brief Agent store module for persisting and loading agent state
 * 
 * This module handles saving and loading agent state to/from persistent storage.
 * It manages the agency.agerun file format and ensures data integrity during
 * persistence operations.
 */

/* Constants */
#define AGENT_STORE_FILE_NAME "agency.agerun"
#define AGENT_STORE_BACKUP_EXT ".bak"

/**
 * Save all agents to persistent storage
 * @return true if successful, false otherwise
 * @note Creates a backup of existing file before saving
 * @note Ownership: Does not affect ownership of any agent resources
 */
bool ar_agent_store__save(void);

/**
 * Load all agents from persistent storage
 * @return true if successful, false otherwise
 * @note Creates agents with their saved state
 * @note Ownership: Creates new agents with their own resources
 */
bool ar_agent_store__load(void);

/**
 * Check if agent store file exists
 * @return true if store file exists, false otherwise
 */
bool ar_agent_store__exists(void);

/**
 * Delete the agent store file
 * @return true if successful or file didn't exist, false on error
 * @note Creates a backup before deletion
 */
bool ar_agent_store__delete(void);

/**
 * Get the path to the agent store file
 * @return Path to the store file (constant string)
 */
const char* ar_agent_store__get_path(void);

#endif /* AGERUN_AGENT_STORE_H */

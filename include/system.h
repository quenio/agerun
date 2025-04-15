#ifndef AGERUN_H
#define AGERUN_H

#include <stdbool.h>
#include <stdint.h>

typedef int64_t agent_id_t;
typedef int32_t version_t;

/**
 * Initialize the Agerun runtime system
 * @param method_name Name of the initial method to run
 * @param version Version of the method (0 for latest)
 * @return ID of the created initial agent, or 0 on failure
 */
agent_id_t agerun_init(const char *method_name, version_t version);

/**
 * Shut down the Agerun runtime system
 */
void agerun_shutdown(void);

/**
 * Define a new method with the given instructions
 * @param name Method name
 * @param instructions The method implementation code
 * @param previous_version Previous version number (0 for new method)
 * @param backward_compatible Whether the method is backward compatible
 * @param persist Whether agents using this method should persist
 * @return New version number, or 0 on failure
 */
version_t agerun_method(const char *name, const char *instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist);

/**
 * Create a new agent instance
 * @param method_name Name of the method to use
 * @param version Version of the method (0 for latest)
 * @param context Context dictionary (NULL for empty)
 * @return Unique agent ID, or 0 on failure
 */
agent_id_t agerun_create(const char *method_name, version_t version, 
                         void *context);

/**
 * Destroy an agent instance
 * @param agent_id ID of the agent to destroy
 * @return true if successful, false otherwise
 */
bool agerun_destroy(agent_id_t agent_id);

/**
 * Send a message to an agent
 * @param agent_id ID of the agent to send to
 * @param message Message content
 * @return true if successful, false otherwise
 */
bool agerun_send(agent_id_t agent_id, const char *message);

/**
 * Process the next pending message in the system
 * @return true if a message was processed, false if no messages
 */
bool agerun_process_next_message(void);

/**
 * Process all pending messages in the system
 * @return Number of messages processed
 */
int agerun_process_all_messages(void);

/**
 * Check if an agent exists
 * @param agent_id ID of the agent to check
 * @return true if the agent exists, false otherwise
 */
bool agerun_agent_exists(agent_id_t agent_id);

/**
 * Get the current number of active agents
 * @return Number of active agents
 */
int agerun_count_agents(void);

/**
 * Save all persistent agents to disk
 * @return true if successful, false otherwise
 */
bool agerun_save_agents(void);

/**
 * Load all persistent agents from disk
 * @return true if successful, false otherwise
 */
bool agerun_load_agents(void);

/**
 * Save all method definitions to disk
 * @return true if successful, false otherwise
 */
bool agerun_save_methods(void);

/**
 * Load all method definitions from disk
 * @return true if successful, false otherwise
 */
bool agerun_load_methods(void);

/**
 * Set a value in memory
 * @param memory Memory dictionary
 * @param key Key to set
 * @param value Value to set
 * @return true if successful, false otherwise
 */
bool memory_set(void *memory, const char *key, void *value);

#endif /* AGERUN_H */

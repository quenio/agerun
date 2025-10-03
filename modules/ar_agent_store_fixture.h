#ifndef AGERUN_AGENT_STORE_FIXTURE_H
#define AGERUN_AGENT_STORE_FIXTURE_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"

/**
 * @file ar_agent_store_fixture.h
 * @brief Agent store fixture module for AgeRun agent persistence testing
 * 
 * This module provides a complete test fixture abstraction that encapsulates
 * agent store, registry, and methodology components. Following Parnas principles,
 * all internal state is hidden behind a facade interface, providing cohesive
 * test operations without exposing implementation details.
 */

typedef struct ar_agent_store_fixture_s ar_agent_store_fixture_t;

/**
 * Creates fixture with test methods (echo, calculator) loaded
 * @return A newly created fixture with full test environment
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
ar_agent_store_fixture_t* ar_agent_store_fixture__create_full(void);

/**
 * Creates fixture with empty methodology (no methods loaded)
 * @return A newly created fixture with minimal environment
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
ar_agent_store_fixture_t* ar_agent_store_fixture__create_empty(void);

/**
 * Destroys fixture and all internal components
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture
 */
void ar_agent_store_fixture__destroy(ar_agent_store_fixture_t *own_fixture);

/**
 * Creates and registers an agent with specified method
 * @param mut_fixture The fixture managing the agent
 * @param ref_method_name Name of method to use
 * @param ref_method_version Version of method to use
 * @return The allocated agent ID, or 0 on failure
 */
int64_t ar_agent_store_fixture__create_agent(ar_agent_store_fixture_t *mut_fixture, const char *ref_method_name, const char *ref_method_version);

/**
 * Gets mutable memory for an agent
 * @param mut_fixture The fixture managing the agent
 * @param agent_id ID of the agent
 * @return Mutable reference to agent memory, or NULL if not found
 */
ar_data_t* ar_agent_store_fixture__get_agent_memory(ar_agent_store_fixture_t *mut_fixture, int64_t agent_id);

/**
 * Verifies agent exists with expected method
 * @param ref_fixture The fixture managing the agent
 * @param agent_id ID of the agent to verify
 * @param ref_expected_method_name Expected method name
 * @return true if agent exists with expected method
 */
bool ar_agent_store_fixture__verify_agent(const ar_agent_store_fixture_t *ref_fixture, int64_t agent_id, const char *ref_expected_method_name);

/**
 * Destroys a single agent
 * @param mut_fixture The fixture managing the agent
 * @param agent_id ID of agent to destroy
 */
void ar_agent_store_fixture__destroy_agent(ar_agent_store_fixture_t *mut_fixture, int64_t agent_id);

/**
 * Destroys multiple agents
 * @param mut_fixture The fixture managing the agents
 * @param agent_ids Array of agent IDs to destroy
 * @param count Number of agents in array
 */
void ar_agent_store_fixture__destroy_agents(ar_agent_store_fixture_t *mut_fixture, const int64_t *agent_ids, int count);

/**
 * Gets count of registered agents
 * @param ref_fixture The fixture to query
 * @return Number of agents currently registered
 */
int64_t ar_agent_store_fixture__get_agent_count(const ar_agent_store_fixture_t *ref_fixture);

/**
 * Saves agent state to persistent storage
 * @param mut_fixture The fixture managing the store
 * @return true if save succeeded
 */
bool ar_agent_store_fixture__save(ar_agent_store_fixture_t *mut_fixture);

/**
 * Loads agent state from persistent storage
 * @param mut_fixture The fixture managing the store
 * @return true if load succeeded
 */
bool ar_agent_store_fixture__load(ar_agent_store_fixture_t *mut_fixture);

/**
 * Deletes the persistent store file
 * @param mut_fixture The fixture managing the store
 */
void ar_agent_store_fixture__delete_file(ar_agent_store_fixture_t *mut_fixture);

/**
 * Gets path to persistent store file
 * @param ref_fixture The fixture managing the store
 * @return Path to store file
 */
const char* ar_agent_store_fixture__get_store_path(const ar_agent_store_fixture_t *ref_fixture);

/**
 * Creates YAML file with single test agent (ID 42, echo method)
 * @param ref_fixture The fixture (unused, for consistency)
 * @param ref_path Path where YAML file should be written
 * @return true if YAML file was created successfully
 */
bool ar_agent_store_fixture__create_yaml_file_single(const ar_agent_store_fixture_t *ref_fixture, const char *ref_path);

/**
 * Creates YAML file with multiple test agents (IDs 10, 20, 30)
 * @param ref_fixture The fixture (unused, for consistency)
 * @param ref_path Path where YAML file should be written
 * @return true if YAML file was created successfully
 */
bool ar_agent_store_fixture__create_yaml_file(const ar_agent_store_fixture_t *ref_fixture, const char *ref_path);

/**
 * Gets first agent ID in registry
 * @param ref_fixture The fixture to query
 * @return First agent ID, or 0 if no agents
 */
int64_t ar_agent_store_fixture__get_first_agent_id(const ar_agent_store_fixture_t *ref_fixture);

/**
 * Gets next agent ID that would be allocated
 * @param ref_fixture The fixture to query
 * @return Next agent ID to be allocated
 */
int64_t ar_agent_store_fixture__get_next_agent_id(const ar_agent_store_fixture_t *ref_fixture);

#endif /* AGERUN_AGENT_STORE_FIXTURE_H */

#ifndef AGERUN_AGENT_STORE_FIXTURE_H
#define AGERUN_AGENT_STORE_FIXTURE_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_agent_store.h"
#include "ar_agent_registry.h"
#include "ar_methodology.h"

/**
 * @file ar_agent_store_fixture.h
 * @brief Agent store fixture module for AgeRun agent persistence testing
 * 
 * This module provides a proper abstraction for agent store test setup
 * and verification operations, following Parnas principles by hiding
 * implementation details and providing a cohesive interface for test
 * management.
 */

typedef struct ar_agent_store_fixture_s ar_agent_store_fixture_t;

/**
 * Creates a new agent store test fixture
 * @return A newly created fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
ar_agent_store_fixture_t* ar_agent_store_fixture__create(void);

/**
 * Destroys a test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture
 */
void ar_agent_store_fixture__destroy(ar_agent_store_fixture_t *own_fixture);

/**
 * Creates a test methodology with echo and calculator methods
 * @param mut_fixture The fixture to populate
 * @return The methodology with registered methods
 * @note Ownership: Returns an owned methodology that caller must destroy
 */
ar_methodology_t* ar_agent_store_fixture__create_test_methodology(ar_agent_store_fixture_t *mut_fixture);

/**
 * Creates YAML content with multiple test agents
 * @param ref_fixture The fixture to use
 * @param ref_store_path Path where YAML file should be written
 * @return true if YAML file was created successfully
 */
bool ar_agent_store_fixture__create_multiple_agents_yaml(const ar_agent_store_fixture_t *ref_fixture, const char *ref_store_path);

/**
 * Verifies an agent has the expected method
 * @param ref_registry Registry containing the agent
 * @param agent_id ID of the agent to verify
 * @param ref_expected_method_name Expected method name
 * @return true if agent exists and has expected method
 */
bool ar_agent_store_fixture__verify_agent(const ar_agent_registry_t *ref_registry, int64_t agent_id, const char *ref_expected_method_name);

/**
 * Destroys all agents from registry
 * @param mut_registry Registry containing agents to destroy
 * @param agent_ids Array of agent IDs to destroy
 * @param count Number of agents in array
 */
void ar_agent_store_fixture__destroy_all_agents(ar_agent_registry_t *mut_registry, const int64_t *agent_ids, int count);

#endif /* AGERUN_AGENT_STORE_FIXTURE_H */

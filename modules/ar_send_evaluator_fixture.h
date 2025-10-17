#ifndef AGERUN_SEND_EVALUATOR_FIXTURE_H
#define AGERUN_SEND_EVALUATOR_FIXTURE_H

#include "ar_send_instruction_evaluator.h"
#include "ar_frame.h"

/**
 * Opaque type for send evaluator fixture
 */
typedef struct ar_send_evaluator_fixture_s ar_send_evaluator_fixture_t;

/**
 * Creates a send evaluator fixture
 * @param ref_test_name Name of the test for identification
 * @return A newly created fixture
 * @note Ownership: Returns owned fixture that caller must destroy
 */
ar_send_evaluator_fixture_t* ar_send_evaluator_fixture__create(
    const char *ref_test_name
);

/**
 * Destroys a send evaluator fixture
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys fixture and all resources
 */
void ar_send_evaluator_fixture__destroy(
    ar_send_evaluator_fixture_t *own_fixture
);

/**
 * Creates a send evaluator from the fixture
 * @param ref_fixture The fixture to use
 * @return A newly created send evaluator
 * @note Ownership: Returns owned evaluator that caller must destroy
 */
ar_send_instruction_evaluator_t* ar_send_evaluator_fixture__create_evaluator(
    const ar_send_evaluator_fixture_t *ref_fixture
);

/**
 * Creates a frame for evaluation
 * @param ref_fixture The fixture to query
 * @return A frame (borrowed reference)
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_frame_t* ar_send_evaluator_fixture__create_frame(
    ar_send_evaluator_fixture_t *ref_fixture
);

/**
 * Gets the delegation instance
 * @param ref_fixture The fixture to query
 * @return The delegation instance (borrowed reference)
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_delegation_t* ar_send_evaluator_fixture__get_delegation(
    const ar_send_evaluator_fixture_t *ref_fixture
);

/**
 * Gets the agency instance
 * @param ref_fixture The fixture to query
 * @return The agency instance (borrowed reference)
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_agency_t* ar_send_evaluator_fixture__get_agency(
    const ar_send_evaluator_fixture_t *ref_fixture
);

/**
 * Creates and registers a delegate
 * @param ref_fixture The fixture to use
 * @param delegate_id The ID for the delegate (negative)
 * @param ref_name The name for the delegate
 * @return The created delegate (borrowed reference), or NULL on failure
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_delegate_t* ar_send_evaluator_fixture__create_and_register_delegate(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t delegate_id,
    const char *ref_name
);

/**
 * Creates and registers an agent
 * @param ref_fixture The fixture to use
 * @param agent_id The ID for the agent (positive)
 * @param ref_behavior The behavior file path (can be NULL)
 * @return The created agent (borrowed reference), or NULL on failure
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_agent_t* ar_send_evaluator_fixture__create_and_register_agent(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t agent_id,
    const char *ref_behavior
);

#endif /* AGERUN_SEND_EVALUATOR_FIXTURE_H */

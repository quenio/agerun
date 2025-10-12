#ifndef AGERUN_DELEGATION_H
#define AGERUN_DELEGATION_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_log.h"
#include "ar_delegate_registry.h"
#include "ar_delegate.h"

/* Delegation type */
typedef struct ar_delegation_s ar_delegation_t;

/**
 * Create a new delegation instance
 * @param ref_log The log instance to use (borrowed reference)
 * @return New delegation instance (ownership transferred), or NULL on failure
 * @note Ownership: Caller must destroy the returned delegation. Delegation stores
 *       ref_log as borrowed reference.
 */
ar_delegation_t* ar_delegation__create(ar_log_t *ref_log);

/**
 * Destroy a delegation instance
 * @param own_delegation The delegation to destroy (ownership transferred)
 * @note Ownership: Destroys all delegate resources and the delegation itself.
 */
void ar_delegation__destroy(ar_delegation_t *own_delegation);

/**
 * Get the registry from a delegation instance
 * @param ref_delegation The delegation instance (borrowed reference)
 * @return The delegate registry (borrowed reference), or NULL if not initialized
 * @note Ownership: Returns a borrowed reference - do not destroy
 */
ar_delegate_registry_t* ar_delegation__get_registry(const ar_delegation_t *ref_delegation);

/**
 * Register a delegate with the delegation
 * @param mut_delegation The delegation instance (mutable reference)
 * @param delegate_id The delegate ID (negative by convention)
 * @param own_delegate The delegate to register (ownership transferred on success)
 * @return true if successful, false otherwise
 * @note Ownership: Takes ownership of own_delegate on success, caller must destroy on failure
 */
bool ar_delegation__register_delegate(ar_delegation_t *mut_delegation,
                                       int64_t delegate_id,
                                       ar_delegate_t *own_delegate);

#endif /* AGERUN_DELEGATION_H */

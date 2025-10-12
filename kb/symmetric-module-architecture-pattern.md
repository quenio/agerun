# Symmetric Module Architecture Pattern

## Learning
When creating sister modules (like ar_agency and ar_delegation), they should mirror each other's architecture completely, not just partially. If one module has message queues, the sister module needs them too. This symmetric pattern prevents incomplete implementations that block later integration.

## Importance
Partial symmetry creates integration nightmares. In this session, delegates were created without message queues while agents had them. This would have blocked TDD Cycle 7 (message routing) because there would be nowhere to deliver messages to delegates. User insight caught this before implementation started, preventing costly rework.

## Example
```c
// Pattern: ar_agent and ar_agency (the model)
// vs ar_delegate and ar_delegation (the sister modules)

// AGENT STRUCTURE (modules/ar_agent.c:15-24):
struct ar_agent_s {
    int64_t id;
    const ar_method_t *ref_method;
    bool is_active;
    ar_list_t *own_message_queue;  // Message queue infrastructure
    ar_data_t *own_memory;
    const ar_data_t *ref_context;
};

// DELEGATE STRUCTURE (should mirror agent):
struct ar_delegate_s {
    ar_log_t *ref_log;
    const char *type;
    ar_list_t *own_message_queue;  // EXAMPLE: To be added - mirrors agent
};

// AGENCY FUNCTIONS (modules/ar_agency.c):
bool ar_agency__send_to_agent(ar_agency_t*, int64_t agent_id, ar_data_t *own_message);
bool ar_agency__agent_has_messages(ar_agency_t*, int64_t agent_id);
ar_data_t* ar_agency__get_agent_message(ar_agency_t*, int64_t agent_id);

// DELEGATION FUNCTIONS (must mirror agency):
bool ar_delegation__send_to_delegate(ar_delegation_t*, int64_t delegate_id, ar_data_t *own_message);  // EXAMPLE: To be implemented
bool ar_delegation__delegate_has_messages(ar_delegation_t*, int64_t delegate_id);  // EXAMPLE: To be implemented
ar_data_t* ar_delegation__get_delegate_message(ar_delegation_t*, int64_t delegate_id);  // EXAMPLE: To be implemented
```

## Generalization
Symmetric module checklist when creating sister modules:

**1. Data Structures**
- [ ] Core structure fields (id, state, queues, memory)
- [ ] Registry structures (tracking, lookup maps)
- [ ] Supporting types (enums, constants)

**2. Lifecycle Functions**
- [ ] Creation functions (with same parameter patterns)
- [ ] Destruction functions (with same cleanup order)
- [ ] Initialization and reset functions

**3. Core Operations**
- [ ] Message handling (send, has_messages, get_message)
- [ ] State queries (is_active, get_state)
- [ ] Registration/unregistration

**4. Infrastructure**
- [ ] Logging integration
- [ ] Error handling
- [ ] Memory management patterns
- [ ] Ownership semantics

**5. Module Organization**
- [ ] Header file structure
- [ ] Implementation file structure
- [ ] Test file organization
- [ ] Documentation files

## Implementation
Systematic symmetry verification:

```bash
#!/bin/bash
# Verify symmetric modules match patterns

MODULE1="ar_agency"
MODULE2="ar_delegation"
ENTITY1="agent"
ENTITY2="delegate"

echo "=== Checking Symmetric Functions ==="

# Find all public functions in module 1
FUNCTIONS1=$(grep "^${MODULE1}__" modules/${MODULE1}.h | sed 's/(.*//' | sed 's/.* //')

for func1 in $FUNCTIONS1; do
    # Construct expected symmetric function name
    func2=$(echo "$func1" | sed "s/${ENTITY1}/${ENTITY2}/g")

    # Check if it exists
    if grep -q "^${func2}" modules/${MODULE2}.h; then
        echo "✓ $func1 ↔ $func2"
    else
        echo "✗ MISSING: $func2 (to mirror $func1)"
    fi
done

echo ""
echo "=== Checking Structure Fields ==="

# Extract structure fields from module 1
grep -A 20 "struct ${MODULE1}_s {" modules/${MODULE1}.c | grep ";" | sed 's/^[[:space:]]*//'

echo ""
echo "Compare with module 2:"
grep -A 20 "struct ${MODULE2}_s {" modules/${MODULE2}.c | grep ";" | sed 's/^[[:space:]]*//'
```

Pattern matching for implementation:
```c
// When implementing delegate function, find agent equivalent:

// 1. Locate agent pattern
grep -A 20 "ar_agent__send" modules/ar_agent.c

// 2. Copy pattern structure
// 3. Rename: agent → delegate, ar_agent → ar_delegate
// 4. Verify ownership semantics match
// 5. Test

// Document the pattern match in TODO.md:
// ar_agent__send() (agent.c:96) ↔ delegate_send() - takes ownership, adds to queue  // EXAMPLE: Pattern mapping
```

## Related Patterns
- [Module Consistency Verification](module-consistency-verification.md)
- [Implementation Plan Line References](implementation-plan-line-references.md)

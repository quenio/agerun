# Implementation Plan Line References

## Learning
Implementation documentation that includes specific line number references to proven patterns dramatically reduces implementation time by eliminating the "hunt for the pattern" phase. Instead of "follow ar_agent pattern", use "Pattern: ar_agent.c:96-150 (takes ownership, adds to queue via ar_list__add_last)".

## Importance
Generic implementation plans ("implement like agents do") force future implementers to search through code finding the right pattern. This wastes time and risks copying the wrong pattern. Line references provide direct navigation to canonical implementations, ensuring consistency and reducing implementation time by 50%+.

## Example
```markdown
# BAD: Generic reference
**GREEN Phase**: Implement delegate send following the agent pattern  // EXAMPLE: Too vague

# GOOD: Specific line reference
**GREEN Phase**: Implement delegate send function  // EXAMPLE: Future implementation
  - Pattern: ar_agent.c:96-150 (takes ownership, adds to queue via ar_list__add_last)
  - Ownership: Takes ownership of message, stores in queue owned by delegate
  - Returns false and destroys message if queue doesn't exist

# Even better: With multiple pattern points
**GREEN Phase**: Implement delegation routing function  // EXAMPLE: Future implementation
  - Pattern: ar_agency.c:258-302 (finds agent in registry, calls agent send)
  - Line 293: Finds entity via registry lookup
  - Line 301: Calls entity send - ownership transferred
  - Lines 262-265: Destroys message if entity not found
  - Returns true on success, false on failure
```

Real example from ar_agent.c used in implementation plan:

```c
// ar_agent.c:96-150 - The pattern being referenced
bool ar_agent__send(ar_agent_t *mut_agent, ar_data_t *own_message) {
    if (!mut_agent || !own_message) {
        // Destroy the message if we have one but no agent
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }

    // DEBUG: Log direct agent sends (internal)
    fprintf(stderr, "DEBUG [AGENT_SEND]: Direct send to agent id=%lld, ",
            (long long)mut_agent->id);
    // ... debug output ...

    if (!mut_agent->own_message_queue) {
        // If agent has no message queue, destroy the message
        ar_data__destroy_if_owned(own_message, mut_agent);
        return false;
    }

    // Take ownership
    ar_data__take_ownership(own_message, mut_agent);

    // Agent module adds to the queue
    bool result = ar_list__add_last(mut_agent->own_message_queue, own_message);

    // If we couldn't add to the queue, destroy the message
    if (!result) {
        ar_data__destroy_if_owned(own_message, mut_agent);
        return false;
    }

    return true;
}
```

## Generalization
**Line Reference Documentation Standard**:

### For Functions:
```markdown
Function: ar_module__function_name()  // EXAMPLE: Generic template
- Pattern: ar_source_module.c:start-end (brief description)
- Key lines:
  - Line X: Critical operation 1
  - Line Y: Critical operation 2
- Ownership: Explicit ownership semantics
- Returns: What it returns and when
```

### For Structures:
```markdown
Structure: ar_delegate_s
- Pattern: ar_agent.c:15-24 (agent structure with same fields)
- Add field: ar_list_t *own_message_queue (line 21 in agent)
- Cleanup pattern: ar_agent.c:79-89 (destroy messages then queue)
```

### For Patterns:
```markdown
Pattern Name: Message Queue Implementation
- Structure: ar_agent.c:21 (own_message_queue field)
- Init: ar_agent.c:46-47 (ar_list__create)
- Send: ar_agent.c:96-150 (ownership + ar_list__add_last)
- Has: ar_agent.c:221-227 (ar_list__count > 0)
- Get: ar_agent.c:229-242 (ar_list__remove_first + drop ownership)
- Cleanup: ar_agent.c:79-89 (destroy messages + destroy list)
```

## Implementation
How to create line-referenced documentation:

```bash
# 1. Find the canonical pattern implementation
grep -n "ar_agent__send" modules/ar_agent.c
# Output: 96:bool ar_agent__send(ar_agent_t *mut_agent, ar_data_t *own_message) {

# 2. Count lines to end of function
sed -n '96,200p' modules/ar_agent.c | grep -n "^}"
# Output: 55:}
# Therefore: lines 96-150

# 3. Extract key operations
sed -n '96,150p' modules/ar_agent.c | grep -n "ar_list__\|ar_data__take_ownership"
# Note the line numbers of critical operations

# 4. Document with specific references:
# Pattern: ar_agent.c:96-150
# - Line 138: ar_data__take_ownership(own_message, mut_agent)
# - Line 141: ar_list__add_last(mut_agent->own_message_queue, own_message)

# 5. Add to implementation plan in TODO.md
```

Benefits of line references:
1. **Navigation**: Jump directly to canonical code
2. **Precision**: No ambiguity about which pattern to follow
3. **Verification**: Easy to check if implementation matches pattern
4. **Maintenance**: When pattern changes, line reference shows what changed
5. **Learning**: Implementers see the proven pattern immediately

## Related Patterns
- [Symmetric Module Architecture Pattern](symmetric-module-architecture-pattern.md)
- [Validated Documentation Examples](validated-documentation-examples.md)

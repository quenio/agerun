# Global Instance Wrapper Anti-Pattern

## Learning
Creating a "global instance wrapper" that tries to bridge between global state and instance-based APIs by sharing ownership of resources leads to segmentation faults and ownership violations. The wrapper pattern shown in global-to-instance-api-migration.md only works when the global instance truly owns its resources.

## Importance
This anti-pattern violates fundamental ownership principles in C. When resources like interpreters and logs are owned by global state, creating a fake instance that pretends to own them creates dangerous shared ownership situations leading to crashes, double-frees, and use-after-free bugs.

## Example
```c
// ANTI-PATTERN: Dangerous shared ownership
static ar_interpreter_t *g_interpreter = NULL;  // Owned by global state
static ar_log_t *g_log = NULL;                  // Owned by global state
// Using ar_data_t as placeholder for the system instance type
static ar_data_t *g_default_system = NULL;  // EXAMPLE: Would be ar_system_t* after refactoring

// WRONG: This creates shared ownership!
static ar_data_t* _get_global_instance(void) {  // EXAMPLE: Anti-pattern (would return ar_system_t*)
    if (!g_default_system && is_initialized && g_interpreter && g_log) {
        g_default_system = ar_data__create_map();  // EXAMPLE: Simulating system instance creation
        if (g_default_system) {
            // DANGER: Storing pointers that are still owned by global state!
            ar_data__set_map_data(g_default_system, "interpreter", ar_data__create_integer((int64_t)g_interpreter));  // EXAMPLE: Not truly owned!
            ar_data__set_map_data(g_default_system, "log", ar_data__create_integer((int64_t)g_log));  // EXAMPLE: Not truly owned!
        }
    }
    return g_default_system;
}

// This will crash when destroy tries to free resources it doesn't own
void ar_system__destroy_instance(ar_data_t *own_system) {  // EXAMPLE: Hypothetical destroy function
    ar_data_t *ref_interpreter = ar_data__get_map_data(own_system, "interpreter");
    if (ref_interpreter) {
        // DANGER: This would crash - trying to free resources not owned by instance
        ar_interpreter__destroy((ar_interpreter_t*)(intptr_t)ar_data__get_integer(ref_interpreter));  // EXAMPLE: CRASH!
    }
    ar_data__destroy(own_system);
}

// CORRECT: Keep instance and global completely separate
// Note: This shows the old global API pattern (now removed)
// In current code, use ar_system__process_next_message_with_instance()
bool process_next_message_global(void) {  // EXAMPLE: Old pattern
    if (!is_initialized) {
        return false;
    }
    // Direct implementation using global state
    // No fake instance, no shared ownership
    ar_data_t *own_message = ar_agency__get_agent_message(agent_id);
    if (own_message) {
        ar_interpreter__execute_method(g_interpreter, agent_id, own_message);
        ar_data__destroy(own_message);
        return true;
    }
    return false;
}

// CORRECT: Instance owns its own resources
ar_data_t* ar_system__create_instance(ar_agency_t *ref_agency) {  // EXAMPLE: Future create function
    ar_data_t *own_system = ar_data__create_map();
    if (!own_system) {
        return NULL;
    }
    
    // Create instance-owned resources
    ar_log_t *own_log = ar_log__create();
    ar_interpreter_t *own_interpreter = ar_interpreter__create(own_log);
    
    // Store owned resources in map
    ar_data__set_map_data(own_system, "log", ar_data__create_integer((int64_t)own_log));
    ar_data__set_map_data(own_system, "interpreter", ar_data__create_integer((int64_t)own_interpreter));
    ar_data__set_map_data(own_system, "agency", ar_data__create_integer((int64_t)ref_agency));  // Borrowed ref OK
    
    return own_system;
}
```

## Generalization
When migrating from global to instance-based APIs:
1. **Never share ownership** - Resources must be either global-owned OR instance-owned, never both
2. **Keep patterns separate** - Global functions use global state directly, instance functions use instance state
3. **No fake instances** - Don't create wrapper instances that pretend to own global resources
4. **Clear ownership boundaries** - Use naming conventions (own_ vs ref_) to make ownership explicit
5. **Instance independence** - Each instance must own its resources independently of global state
6. **Single global instance pattern** - When needed, use a single g_system that owns everything and delegates

## Implementation
To safely migrate modules with shared resources:
```c
// 1. Keep global API using global state directly
bool system_process_next_message_example(void) {
    // Direct use of global g_interpreter, g_log
    // No wrapper instance needed
}

// 2. Create truly independent instances
ar_data_t* ar_system__create_instance(ar_agency_t *ref_agency) {  // EXAMPLE: Future instance creation
    // Create new interpreter and log for this instance
    // Complete ownership separation from global state
}

// 3. Use delegation only for simple forwarding
int ar_agency__count_agents(void) {
    // This works because agency's global instance owns its registry
    ar_agency_t *ref_agency = ar_agency__get_global_instance();  // EXAMPLE: Safe delegation
    return ref_agency ? ar_agency__count_agents_with_instance(ref_agency) : 0;
}

// 4. CORRECT: Single global instance pattern (from ar_system refactoring)
static ar_data_t *g_system = NULL;  // EXAMPLE: Would be ar_system_t* - owns everything

// Note: This shows the old global API pattern (now removed)
// In current code, only instance-based APIs exist
int64_t init_global_example(const char *ref_method_name, const char *ref_version) {  // EXAMPLE: Old pattern
    if (g_system) {
        printf("Already initialized\n");
        return 0;
    }
    
    // Create single global instance that owns everything
    g_system = ar_system__create(NULL);  // EXAMPLE: ar_system__create returns ar_system_t*
    if (!g_system) {
        return 0;
    }
    
    // Delegate to instance function
    return ar_system__init_with_instance(g_system, ref_method_name, ref_version);  // EXAMPLE: Safe delegation
}
```

## Related Patterns
- [Global to Instance API Migration](global-to-instance-api-migration.md)
- [Module Instantiation Prerequisites](module-instantiation-prerequisites.md)
- [Instance Association Pattern](instance-association-pattern.md)
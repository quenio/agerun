# Dependency Management Examples

## Learning

This pattern provides concrete examples and commands for preventing circular dependencies and managing module relationships in the codebase.

## Importance

Circular dependencies make code hard to understand, test, and maintain. They violate Parnas principles and lead to automatic PR rejection. These examples show how to detect and prevent them.

## Example

### Detecting Dependencies
```bash
# Check before adding includes:
grep -n "#include.*ar_" module.h module.c

# Example output showing potential issue:
ar_agent.h:5:#include "ar_system.h"  # agent depends on system
ar_system.h:3:#include "ar_agent.h"  # system depends on agent - CIRCULAR!
```

### Proper Hierarchy
```
Foundation Layer (no dependencies on higher layers):
- ar_io.h, ar_list.h, ar_map.h

Data Layer (depends only on Foundation):
- ar_heap.h (exception: mutual with ar_io)
- ar_data.h

Core Layer (depends on Foundation + Data):
- ar_agent.h, ar_method.h

System Layer (depends on all lower layers):
- ar_agency.h, ar_interpreter.h
```

## Generalization

### Dependency Rules

**Allowed Dependencies**:
- Higher layers can depend on lower layers
- Same layer modules can depend on each other (carefully)
- Single accepted exception: heap ↔ io circular

**Forbidden Patterns**:
- Module A→B→C→A cycles
- Lower layers depending on higher layers
- Parser depending on executor (should be separate)

### Code Duplication Prevention

**Before Writing New Code**:
```bash
# Search for existing functionality
grep -r "function_name\|concept" modules/

# Check for similar patterns
grep -r "similar_function" modules/*.c

# Look for _for_int, _for_string patterns (smell)
grep -r "_for_int\|_for_string" modules/
```

**DRY Process**:
1. Stop before copying
2. Extract common functions
3. Use data tables for variations
4. Parameterize differences
5. Verify with diff after moving

## Implementation

### Dependency Checking Commands

```bash
# Full dependency check for a module
MODULE="ar_agent"
echo "=== Checking $MODULE dependencies ==="
echo "Direct includes:"
grep "^#include.*ar_" modules/${MODULE}.h modules/${MODULE}.c

echo "Who depends on $MODULE:"
grep -l "#include.*${MODULE}.h" modules/*.h modules/*.c

echo "What $MODULE depends on:"
grep "^#include.*ar_" modules/${MODULE}.c | cut -d'"' -f2 | sort -u
```

### Architectural Patterns for Clean Dependencies

**Pattern Hierarchy** (prefer higher ones):
1. Interface segregation - Split large interfaces
2. Registry pattern - Central registration point
3. Facade pattern - Coordinate only, no logic
4. Parser/Executor separation - Parse once, execute many
5. Callbacks - Last resort for unavoidable coupling

### Refactoring Circular Dependencies

**Step 1: Identify the Cycle**
```bash
# Visual dependency graph
for f in modules/*.h; do
    echo "$(basename $f):"
    grep "^#include.*ar_" $f | sed 's/.*"\(.*\)".*/  -> \1/'
done
```

**Step 2: Break the Cycle**

Option A: Extract common interface
```c
// Before: agent.h includes system.h, system.h includes agent.h

// After: Both include common ar_types.h
// ar_types.h - common types only
typedef struct ar_agent_s ar_agent_t;
typedef struct ar_system_s ar_system_t;
```

Option B: Use forward declarations
```c
// Instead of #include "ar_system.h"
typedef struct ar_system_s ar_system_t;
```

Option C: Dependency injection
```c
// EXAMPLE: Pass dependencies as parameters instead of including
// void module_process(module_type* module, void* context);
```

### Red Flags to Watch For

**Naming Patterns**:
- Functions like `_for_int`, `_for_string` (use generics)
- Repeated validation logic across modules
- Similar function names doing similar things

**Structural Issues**:
- Parsing and execution in same module
- Modules changing for multiple reasons
- Long chains of includes

### Domain-Specific Types

Instead of primitive obsession:
```c
// EXAMPLE: Bad - Primitives everywhere
void process(char* path, int line, char* error);

// EXAMPLE: Good - Domain types
typedef struct ar_path_s ar_path_t;
void process(ar_path_t* path, ar_data_t* error);
```

## Related Patterns

- [Architectural Patterns Hierarchy](architectural-patterns-hierarchy.md)
- [Domain Specific Type Creation](domain-specific-type-creation.md)
- [No Circular Dependencies Principle](no-circular-dependencies-principle.md)
- [Code Smell Duplicate Code](code-smell-duplicate-code.md)
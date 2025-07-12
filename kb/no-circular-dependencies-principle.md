# No Circular Dependencies Principle

## Overview

The No Circular Dependencies principle requires that module dependencies form a strict hierarchy without cycles. Module A cannot depend on Module B if Module B (directly or indirectly) depends on Module A.

## Core Concept

**Definition**: Dependencies between modules must form a directed acyclic graph (DAG). No circular dependencies are allowed.

**Purpose**: Enables independent compilation, testing, and understanding of modules. Prevents tight coupling and makes the system more maintainable.

## AgeRun Dependency Hierarchy

### Foundation Layer
**No dependencies on other AgeRun modules**:
- `ar_io` - File and console I/O operations
- `ar_list` - Dynamic list implementation  
- `ar_map` - Hash map implementation
- `ar_assert` - Debug assertions

### Data Layer  
**Depends only on Foundation**:
- `ar_heap` - Memory management (depends on ar_io for logging)
- `ar_data` - Unified data types (depends on ar_list, ar_map, ar_heap)

### Core Layer
**Depends on Foundation + Data**:
- `ar_string` - String operations
- `ar_semver` - Version parsing
- `ar_expression` - Expression parsing/evaluation
- `ar_instruction` - Instruction parsing/evaluation
- `ar_method` - Method definitions
- `ar_agent` - Agent instances

### System Layer
**Depends on all lower layers**:
- `ar_methodology` - Method management
- `ar_agency` - Agent management  
- `ar_interpreter` - Instruction execution
- `ar_system` - System coordination

## Acceptable Exception

**Single Accepted Cycle**: `ar_heap ↔ ar_io`
- `ar_heap` depends on `ar_io` for leak reporting
- `ar_io` depends on `ar_heap` for memory allocation
- **Justification**: Both are foundation modules with careful design to avoid infinite recursion
- **Documentation**: Explicitly noted and accepted

## Detecting Circular Dependencies

### Manual Detection
```bash
# Check dependencies in header files
grep -n "#include.*ar_" modules/ar_data.h

# Verify dependency direction
grep -r "ar_heap" modules/ar_data.*    # Should find ar_data using ar_heap
grep -r "ar_data" modules/ar_heap.*    # Should NOT find ar_heap using ar_data
```

### Dependency Analysis
```bash
# Generate dependency report
make analyze-deps  # Creates module_dependency_report.md
```

## Common Violations

### Direct Circular Dependencies

**Problem**: Modules directly reference each other
```c
// ar_agent.h
#include "ar_agency.h"  // Agent depends on Agency

// ar_agency.h  
#include "ar_agent.h"   // Agency depends on Agent - CIRCULAR!
```

**Solution**: Extract common interfaces or use forward declarations
```c
// ar_agent.h
typedef struct ar_agency_s ar_agency_t;  // Forward declaration
void ar_agent__register_with_agency(ar_agency_t* agency);

// ar_agency.h
#include "ar_agent.h"  // Only Agency depends on Agent
ar_agent_t* ar_agency__create_agent(const char* method_name);
```

### Indirect Circular Dependencies

**Problem**: Circular dependency through intermediate modules
```c
// A → B → C → A cycle
// ar_expression.h includes ar_data.h
// ar_data.h includes ar_method.h  
// ar_method.h includes ar_expression.h  // CIRCULAR!
```

**Solution**: Restructure to create proper hierarchy
```c
// Fixed hierarchy:
// ar_expression depends on ar_data (expressions use data)
// ar_method depends on ar_expression (methods contain expressions)
// ar_data is independent (foundation data types)
```

### Utility Dependencies

**Problem**: Low-level module depending on high-level utilities
```c
// ar_heap.c (foundation layer)
#include "ar_log.h"  // BAD: Heap depending on logging module
```

**Solution**: Use only foundation dependencies or extract interface
```c
// ar_heap.c - Use only ar_io for output
#include "ar_io.h"
void ar_heap__report_leaks() {
    ar_io__printf("Memory leaks detected\n");  // Direct I/O instead of logging
}
```

## Dependency Management Strategies

### Interface Segregation

**Split large modules** to break circular dependencies:
```c
// Before: Large ar_agency module with circular dependencies

// After: Split into focused modules
// ar_agent_registry.h - Agent creation (depends on ar_agent)
// ar_agent_store.h - Agent persistence (depends on ar_agent)  
// ar_agent_update.h - Agent messaging (depends on ar_agent)
// ar_agency.h - Coordination facade (depends on all registry modules)
```

### Registry Pattern

**Central ownership** eliminates cycles:
```c
// GOOD: Registry owns all agents, agents don't reference registry
// ar_agent_registry.h
ar_agent_t* ar_agent_registry__create_agent(const char* method_name);
void ar_agent_registry__destroy_agent(uint64_t agent_id);

// ar_agent.h - No registry dependencies
void ar_agent__send_message(ar_agent_t* agent, ar_data_t* message);
```

### Callbacks and Dependency Injection

**Last resort** for unavoidable coupling:
```c
// Break cycle with callback pattern
typedef void (*ar_error_callback_t)(const char* message);

// ar_heap.h
void ar_heap__set_error_callback(ar_error_callback_t callback);

// Higher-level module provides callback
void setup_heap_logging() {
    ar_heap__set_error_callback(ar_log__error);
}
```

## Verification Process

### Pre-Change Verification
```bash
# Before adding new #include
grep -n "#include.*ar_" target_module.h target_module.c

# Check if new dependency creates cycle
grep -r "target_module" modules/new_dependency.*
```

### Post-Change Verification  
```bash
# Verify compilation order works
make clean && make

# Check dependency report
make check-deps
```

### Manual Verification
1. **List all includes** in both directions
2. **Trace dependency path** from A to B  
3. **Check reverse path** from B to A
4. **Flag any cycles** for resolution

## Benefits

### Independent Compilation
- Modules can be compiled in dependency order
- Changes have predictable rebuild scope
- Parallel compilation is possible

### Easier Testing
- Mock dependencies are always "lower" in hierarchy
- Unit tests don't require complex setup
- Integration testing follows clear layers

### Better Understanding
- Module relationships are clear
- System architecture is explicit
- Code navigation follows logical flow

## Refactoring to Break Cycles

### Step 1: Identify the Cycle
```bash
# Find circular reference
grep -r "module_a" modules/module_b.*  # Check B→A dependency
grep -r "module_b" modules/module_a.*  # Check A→B dependency
```

### Step 2: Determine Proper Direction
- Which module is more fundamental?
- Which dependency is stronger/more essential?
- Can functionality be moved to break the cycle?

### Step 3: Extract Common Interface
```c
// Extract shared types to separate module
// ar_shared_types.h - No dependencies
typedef struct ar_agent_s ar_agent_t;
typedef struct ar_agency_s ar_agency_t;

// Both modules depend on shared types, not each other
```

### Step 4: Use Forward Declarations
```c
// Instead of including header, use forward declaration
typedef struct ar_complex_type_s ar_complex_type_t;

// Function signatures with opaque types
void ar_module__function(ar_complex_type_t* param);
```

## Related Principles

- **Single Responsibility**: Clear responsibilities help determine proper dependencies
- **Information Hiding**: Reduces need for cross-dependencies
- **Minimal Interfaces**: Fewer dependencies reduce chance of cycles

## Examples

**Good Dependency Structure**:
```c
// Clear hierarchy: Foundation → Data → Core → System
ar_system.h      →  ar_agency.h     →  ar_agent.h      →  ar_data.h       →  ar_heap.h
(System layer)      (Core layer)       (Core layer)       (Data layer)       (Foundation)
```

**Bad Circular Structure**:
```c
// Circular dependencies
ar_agent.h       ↔  ar_agency.h     // Agents and agency reference each other
ar_heap.h        →  ar_log.h        →  ar_data.h       →  ar_heap.h  // Full cycle
```
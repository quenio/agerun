# Object-Oriented to C Adaptation

## Learning
Object-oriented software engineering concepts (code smells, design patterns, principles) can be effectively adapted to procedural C code by mapping classes to modules, objects to structs, and methods to functions.

## Importance
Enables application of well-established software engineering principles to C codebases, provides vocabulary for discussing architectural issues in procedural languages, and bridges modern practices with systems programming.

## Example
Code smells successfully adapted from OO to C:
- **Feature Envy**: C functions accessing other modules' data structures excessively
- **Large Class** → **Large Module**: Modules with too many functions/responsibilities
- **Primitive Obsession**: Using raw strings/ints instead of domain types like `ar_method_id_t`
- **Data Clumps**: Parameters that always appear together → parameter objects

## Generalization
Apply OO-to-C adaptation pattern systematically:
1. **Identify core concept**: What principle/pattern addresses
2. **Map OO elements to C equivalents**:
   - Class → Module (.h/.c pair)
   - Object → Struct instance
   - Method → Function
   - Inheritance → Composition/function pointers
   - Encapsulation → Opaque types + access functions
3. **Create C-specific examples**: Use actual codebase patterns
4. **Adapt detection/refactoring techniques**: Use C-appropriate tools
5. **Maintain conceptual integrity**: Preserve the underlying principle

## Implementation
```c
// OO concept: Class with too many responsibilities
class SystemManager {
    void manageAgents();
    void manageMessages();
    void manageConfig();
}

// C adaptation: Module with too many responsibilities  
// ar_system.h - Large Module smell
bool ar_system__manage_agents(...);
bool ar_system__manage_messages(...);
bool ar_system__manage_config(...);
// Solution: Split into ar_agency.c, ar_messaging.c, ar_config.c

// OO concept: Primitive Obsession
void createAgent(String methodName, String version);

// C adaptation: Same smell, C-specific solution
ar_agent_t* ar_agent__create(const char* name, const char* version);  // Bad
ar_agent_t* ar_agent__create(ar_method_id_t* method_id);              // Good
```

## Related Patterns
- [Systematic Guideline Enhancement](kb/systematic-guideline-enhancement.md)
- [Research-driven Development](kb/research-driven-development.md)
- [Domain-specific Type Creation](kb/domain-specific-type-creation.md)
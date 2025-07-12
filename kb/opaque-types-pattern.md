# Opaque Types Pattern

## Learning
Use opaque types with struct definition in .c file and typedef in .h file to hide implementation details

## Importance
Enforces information hiding, prevents clients from depending on internal structure, enables interface evolution

## Example
```c
// In ar_data.h (public interface)
typedef struct ar_data_s ar_data_t;

// In ar_data.c (implementation only)
struct ar_data_s {
    ar_data_type_t type;
    union {
        int64_t integer;
        double real;
        char *string;
        // ... internal fields
    } value;
};
```

## Generalization
Hide implementation details behind stable interfaces to enable independent evolution of components

## Implementation
- Public header: `typedef struct ar_<module>_s ar_<module>_t;`
- Implementation file: Full `struct ar_<module>_s { ... };` definition
- All access through function interfaces
- Never expose internal fields or struct layout
- Required for complex data structures

## Related Patterns
- Information hiding principle
- Parnas design principles
- Interface stability
- Module independence
- API evolution support
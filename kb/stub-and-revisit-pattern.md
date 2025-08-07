# Stub and Revisit Pattern

## Learning
When implementing features that depend on unavailable infrastructure, comment out the advanced functionality with clear documentation about when it will be enabled, allowing development to continue without blocking.

## Importance
This pattern prevents development stalls while maintaining clear documentation of intended functionality. It creates a traceable path for future enhancement without losing implementation details.

## Example
```c
// Bootstrap method with spawn functionality commented out
// TODO: Re-enable once system can load methods from files (Cycle 5)
/*
memory.echo_context := 0
memory.echo_id := spawn("echo", "1.0.0", memory.echo_context)
send(memory.echo_id, context)
*/

// Simplified version until dependencies available
memory.initialized := if(memory.is_wake > 0, 1, memory.initialized)
memory.status := if(memory.initialized > 0, "Bootstrap initialized", "Bootstrap ready")
send(0, memory.status)
```

## Generalization
Apply this pattern when:
1. A feature requires infrastructure not yet available
2. The dependency timeline is known (e.g., "Cycle 5")
3. Implementation details are clear but execution blocked
4. Development momentum needs to continue

## Implementation
```c
// Pattern structure:
// 1. Comment with clear TODO and dependency reference
// TODO: Re-enable [feature] once [dependency] is available ([timeline])

// 2. Commented implementation preserving logic
/*
[Full implementation that will work once dependency available]
*/

// 3. Simplified placeholder if needed
// [Minimal implementation that works now]

// 4. Test also follows pattern
// TODO: Re-enable once [dependency] available
// test_advanced_feature();
```

## Related Patterns
- [Progressive System Decoupling via TDD](progressive-system-decoupling-tdd.md)
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)
# Evaluator Migration Priority Strategy

## Learning
When migrating multiple similar modules (like evaluators), prioritize them from simple to complex based on objective complexity metrics. This reduces risk by proving patterns on simple cases before tackling complex ones.

## Importance
Starting with the simplest modules first allows you to:
- Validate the migration pattern with minimal risk
- Build confidence and expertise gradually
- Identify and solve common issues early
- Refine the process before tackling complex cases

## Example
During the evaluator C-to-Zig migration, we prioritized based on complexity:

```c
// Simple (1 argument, straightforward logic)
ar_exit_instruction_evaluator_t* evaluator;  // Migrated first as proof of concept

// Moderate (2-3 arguments, some branching)
ar_send_instruction_evaluator_t* send_eval;  // Next priority
ar_assignment_instruction_evaluator_t* assign_eval;  // Simple 2-part structure

// Complex (multiple arguments, string manipulation, parsing)
ar_compile_instruction_evaluator_t* compile_eval;  // 3 string arguments
ar_expression_evaluator_t* expr_eval;  // Multiple type handlers
ar_instruction_evaluator_t* inst_eval;  // Facade coordinating others
```

The migration order was determined by:
1. **Argument count**: Fewer arguments = simpler error paths
2. **Logic complexity**: Linear flow vs branching/loops
3. **Dependencies**: Standalone vs coordinating other modules
4. **String handling**: Integer operations simpler than string building

## Generalization
Apply this strategy to any large-scale migration:

1. **Analyze complexity metrics**:
   - Parameter count
   - Lines of code
   - Cyclomatic complexity
   - External dependencies
   - Error handling paths

2. **Create ordered list**:
   ```bash
   # Count lines and parameters for each module
   for file in modules/*_evaluator.c; do
     echo "$(wc -l < "$file") $(grep -c "ref_.*," "$file") $file"
   done | sort -n
   ```

3. **Start with simplest**:
   - Prove the pattern works
   - Document issues encountered
   - Refine approach

4. **Progress systematically**:
   - Apply lessons learned to next module
   - Update migration guide with new insights
   - Maintain momentum with quick wins

## Implementation
For the AgeRun evaluator migration:

```bash
# Priority order based on complexity analysis:
1. ar_exit_instruction_evaluator      # 1 arg, ~150 lines
2. ar_send_instruction_evaluator      # 2 args, ~180 lines  
3. ar_assignment_instruction_evaluator # 2 parts, ~200 lines
4. ar_deprecate_instruction_evaluator # 3 args, ~220 lines
5. ar_spawn_instruction_evaluator     # 3 args, ~250 lines
6. ar_condition_instruction_evaluator # if logic, ~280 lines
7. ar_build_instruction_evaluator     # string ops, ~300 lines
8. ar_parse_instruction_evaluator     # parsing, ~320 lines
9. ar_compile_instruction_evaluator   # 3 strings, ~350 lines
10. ar_expression_evaluator           # multiple types, ~500 lines
11. ar_instruction_evaluator          # facade, coordinates all
```

This ordering ensures each migration builds on the previous one's success.

## Related Patterns
- [C to Zig Module Migration](c-to-zig-module-migration.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [Zig Defer for Error Cleanup](zig-defer-error-cleanup-pattern.md)
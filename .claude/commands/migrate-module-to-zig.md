# Migrate Module to Zig

Migrate the implementation of the **{{1}}** module from C to Zig following the systematic approach documented in CLAUDE.md and the comprehensive migration guide at kb/c-to-zig-module-migration.md.

## Migration Requirements

1. **Follow the migration guide**: Use kb/c-to-zig-module-migration.md as your primary reference
2. **Maintain C API compatibility**: The Zig implementation must be a drop-in replacement
3. **Apply memory debugging patterns**: Use kb/zig-migration-memory-debugging.md for ownership issues
4. **Zero memory leaks**: Verify with individual test memory reports
5. **Validate documentation**: Ensure all documentation passes `make check-docs`

## Pre-Migration Checklist

Before creating the plan, verify:
- Module exists: `modules/{{1}}.c` and `modules/{{1}}.h`
- Tests exist: `modules/{{1}}_tests.c`
- Current test status: Run `make {{1}}_tests` and check memory report
- Dependencies: Audit with `grep -n "#include.*ar_" modules/{{1}}.h modules/{{1}}.c`

## Plan Structure

Create a detailed migration plan that includes:

### Phase 1: Assessment
- Dependency analysis (which modules does {{1}} depend on?)
- Migration challenges (variadic functions, platform-specific code, etc.)
- Current API documentation review
- Memory ownership patterns identification

### Phase 2: Implementation Strategy
- Zig file creation approach
- Type mapping decisions
- Memory management approach
- Error handling strategy

### Phase 3: Testing Approach
- How to verify behavior matches C implementation
- Memory leak detection strategy
- Documentation validation approach

### Phase 4: Cleanup Plan
- C file deletion timing
- Documentation updates needed
- Verification steps

## Critical Reminders

- **All parameters must be nullable** for C API compatibility
- **Reserved keywords** like 'type' need renaming (e.g., 'node_type')
- **Match C behavior exactly** for ownership transfer functions
- **Use C types directly** to minimize casting
- **Compare implementations line-by-line** when debugging

## User Approval

After creating the plan, ask for user approval with:
"I've created a migration plan for the {{1}} module. This plan follows the systematic C-to-Zig migration guide and includes all necessary verification steps. Would you like me to proceed with the migration?"

Only proceed with implementation after receiving explicit approval.

## Post-Migration Verification

After migration, ensure:
1. All tests pass: `make {{1}}_tests`
2. Zero memory leaks: Check `bin/memory_report_{{1}}_tests.log`
3. Documentation valid: `make check-docs`
4. Clean build: `make clean build`

Remember to follow TDD principles - complete any in-progress development cycles before starting the migration.